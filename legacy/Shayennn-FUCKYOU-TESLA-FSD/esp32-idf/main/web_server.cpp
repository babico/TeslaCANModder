#include "web_server.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cinttypes>

#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "mbedtls/base64.h"

#include "auth.h"
#include "can_fast_path.h"
#include "can_logger.h"
#include "ota.h"

namespace {

constexpr char kLogTag[] = "web";

httpd_handle_t gServer = nullptr;

struct SignalView {
    bool has_driver_assist = false;
    bool has_legacy_mux0 = false;
    bool has_mux0 = false;
    bool has_mux1 = false;
    bool has_mux2 = false;
    bool has_ap_mode = false;
    tesla_fsd::can_frame driver_assist{};
    tesla_fsd::can_frame legacy_mux0{};
    tesla_fsd::can_frame mux0{};
    tesla_fsd::can_frame mux1{};
    tesla_fsd::can_frame mux2{};
    tesla_fsd::can_frame ap_mode{};
};

const char* handlerTypeName(int type) {
    switch (type) {
        case 0: return "Legacy";
        case 1: return "HW3";
        case 2: return "HW4";
        default: return "Unknown";
    }
}

const char* drivingSideName(uint8_t side) {
    switch (side) {
        case tesla_fsd::UI_DRIVING_SIDE_LEFT: return "left";
        case tesla_fsd::UI_DRIVING_SIDE_RIGHT: return "right";
        case tesla_fsd::UI_DRIVING_SIDE_UNKNOWN: return "unknown";
        default: return "invalid";
    }
}

const char* branchName(uint8_t branch) {
    switch (branch) {
        case tesla_fsd::UI_APMV3_BRANCH_LIVE: return "live";
        case tesla_fsd::UI_APMV3_BRANCH_STAGE: return "stage";
        case tesla_fsd::UI_APMV3_BRANCH_DEV: return "dev";
        case tesla_fsd::UI_APMV3_BRANCH_STAGE2: return "stage2";
        case tesla_fsd::UI_APMV3_BRANCH_EAP: return "eap";
        case tesla_fsd::UI_APMV3_BRANCH_DEMO: return "demo";
        default: return "invalid";
    }
}

int decodeAutopilotMode(const tesla_fsd::can_frame& frame) {
    return static_cast<int>((frame.data[5] >> 2) & 0x07u);
}

int decodeFollowDistance(const tesla_fsd::can_frame& frame) {
    return static_cast<int>((frame.data[5] >> 5) & 0x07u);
}

int decodeSpeedOffset(const tesla_fsd::can_frame& frame) {
    return static_cast<int>(((frame.data[0] >> 6) & 0x03u) | ((frame.data[1] & 0x3Fu) << 2));
}

int decodeSpeedProfile(const SignalView& view, int handler_type) {
    switch (handler_type) {
        case 0:
            if (!view.has_legacy_mux0) return -1;
            return static_cast<int>((view.legacy_mux0.data[6] >> 1) & 0x03u);
        case 1:
            if (!view.has_mux0) return -1;
            return static_cast<int>((view.mux0.data[6] >> 1) & 0x03u);
        case 2:
            if (!view.has_mux2) return -1;
            return static_cast<int>((view.mux2.data[7] >> 4) & 0x07u);
        default:
            return -1;
    }
}

SignalView buildSignalView() {
    constexpr size_t kMaxEntries = 128;
    can_logger::Entry entries[kMaxEntries];
    size_t lost = 0;
    size_t count = can_logger::snapshot(entries, kMaxEntries, &lost);
    (void)lost;

    SignalView view;
    for (size_t idx = count; idx > 0; --idx) {
        const tesla_fsd::can_frame& frame = entries[idx - 1].frame;
        if (!view.has_legacy_mux0 && frame.can_id == 1006 && tesla_fsd::readMuxID(frame) == 0) {
            view.legacy_mux0 = frame;
            view.has_legacy_mux0 = true;
            continue;
        }
        if (!view.has_driver_assist && frame.can_id == tesla_fsd::UI_DRIVER_ASSIST_CONTROL_ID) {
            view.driver_assist = frame;
            view.has_driver_assist = true;
            continue;
        }
        if (frame.can_id == tesla_fsd::UI_AUTOPILOT_CONTROL_ID) {
            switch (tesla_fsd::readMuxID(frame)) {
                case 0:
                    if (!view.has_mux0) {
                        view.mux0 = frame;
                        view.has_mux0 = true;
                    }
                    break;
                case 1:
                    if (!view.has_mux1) {
                        view.mux1 = frame;
                        view.has_mux1 = true;
                    }
                    break;
                case 2:
                    if (!view.has_mux2) {
                        view.mux2 = frame;
                        view.has_mux2 = true;
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
        if (!view.has_ap_mode && frame.can_id == 2047 && frame.data[0] == 2u) {
            view.ap_mode = frame;
            view.has_ap_mode = true;
        }
        if (view.has_driver_assist && view.has_legacy_mux0 && view.has_mux0 && view.has_mux1 && view.has_mux2 && view.has_ap_mode) {
            break;
        }
    }
    return view;
}

bool isAuthenticated(httpd_req_t* req) {
    char buf[128] = {};
    if (httpd_req_get_hdr_value_str(req, "Authorization", buf, sizeof(buf)) != ESP_OK) {
        return false;
    }
    static const char kBearer[] = "Bearer ";
    if (std::strncmp(buf, kBearer, sizeof(kBearer) - 1) != 0) return false;
    const char* token = buf + sizeof(kBearer) - 1;
    size_t token_len = std::strlen(token);

    uint8_t response[auth::kHashLen];
    if (token_len != auth::kHashLen * 2) return false;
    for (size_t i = 0; i < auth::kHashLen; ++i) {
        unsigned int v;
        if (std::sscanf(token + i * 2, "%02x", &v) != 1) return false;
        response[i] = static_cast<uint8_t>(v);
    }
    return auth::verify_session_token(response, auth::kHashLen);
}

bool requireAuth(httpd_req_t* req) {
    if (!auth::has_password()) return true;
    if (isAuthenticated(req)) return true;
    httpd_resp_set_status(req, "401 Unauthorized");
    httpd_resp_send(req, "{}", 2);
    return false;
}

static const char kIndexHtml[] = R"(<!DOCTYPE html>
<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Tesla FSD CAN Mod</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:system-ui,sans-serif;background:#1a1a2e;color:#e0e0e0;padding:16px}
h1{font-size:1.4em;color:#0f9;margin-bottom:12px}
.card{background:#16213e;border-radius:8px;padding:12px;margin-bottom:12px}
label{display:block;margin:6px 0 2px;font-size:.85em;color:#aaa}
input,select,button{width:100%;padding:8px;border:1px solid #333;border-radius:4px;background:#0f3460;color:#e0e0e0;font-size:1em;margin-bottom:8px}
button{background:#0f9;border:none;color:#000;font-weight:bold;cursor:pointer;margin-top:4px}
button:disabled{opacity:.5}
pre{background:#0a0a1a;padding:8px;border-radius:4px;font-size:.8em;overflow:auto;max-height:300px}
.status{display:flex;gap:12px;flex-wrap:wrap}
.stat{flex:1;min-width:80px;text-align:center}
.stat .val{font-size:1.5em;color:#0f9}
.stat .lbl{font-size:.7em;color:#888}
#log{font-size:.75em;font-family:monospace}
.err{color:#f44}
</style></head><body>
<h1>Tesla FSD CAN Mod</h1>
<div id='setup' class='card' style='display:none'>
<h2>First Boot Setup</h2>
<label>Set Password</label><input type='password' id='pw1' maxlength='63'>
<label>Confirm Password</label><input type='password' id='pw2' maxlength='63'>
<button onclick='doSetup()'>Set Password</button>
</div>
<div id='login' class='card' style='display:none'>
<h2>Login</h2>
<label>Password</label><input type='password' id='pw' maxlength='63'>
<button onclick='doLogin()'>Login</button>
<button onclick='armRecovery()'>Forgot Password</button>
<p id='recoveryStatus'></p>
</div>
<div id='main' style='display:none'>
<div class='card status' id='stats'></div>
<div class='card status' id='signals'></div>
<div class='card'>
<h2>Configuration</h2>
<label>Handler Type</label>
<select id='handlerType'><option value='0'>Legacy</option><option value='1' selected>HW3</option><option value='2'>HW4</option></select>
<button onclick='setHandler()'>Apply</button>
</div>
<div class='card'>
<h2>CAN Log</h2>
<pre id='log'>Loading...</pre>
<button onclick='refreshLog()'>Refresh</button>
</div>
<div class='card'>
<h2>OTA Update</h2>
<input type='file' id='firmware' accept='.bin'>
<button onclick='doOTA()'>Upload</button>
<p id='otaStatus'></p>
</div>
</div>
<script>
let token=null;
async function api(path,opts){
 const h=opts&&opts.headers?opts.headers:{};
 if(token)h['Authorization']='Bearer '+token;
 const r=await fetch(path,{...opts,headers:h});
 if(r.status===401){token=null;showLogin();return null;}
 return r;
}
function showSetup(){document.getElementById('setup').style.display='block';document.getElementById('login').style.display='none';document.getElementById('main').style.display='none';}
function showLogin(){document.getElementById('setup').style.display='none';document.getElementById('login').style.display='block';document.getElementById('main').style.display='none';}
function showMain(){document.getElementById('setup').style.display='none';document.getElementById('login').style.display='none';document.getElementById('main').style.display='block';refreshStats();refreshLog();}
function fmtNum(v){return v===undefined||v===null||v<0?'-':String(v);}
async function init(){
  const r=await fetch('/api/status');
  const d=await r.json();
  document.getElementById('recoveryStatus').textContent=d.recovery_armed?'Recovery armed for 30s. Toggle FSD on, then off, from the steering wheel.':'';
  document.getElementById('handlerType').value=String(d.handler_type??1);
  if(!d.has_password){showSetup();}
  else if(!token){showLogin();}
  else{showMain();}
}
async function doSetup(){
 const p1=document.getElementById('pw1').value,p2=document.getElementById('pw2').value;
 if(p1!==p2){alert('Passwords do not match');return;}
 if(p1.length<4){alert('Password too short');return;}
  if(p1.length>63){alert('Password too long');return;}
  const r=await fetch('/api/setup',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({password:p1})});
  if(!r.ok){alert('Setup failed');return;}
  doLoginWith(p1);
}
async function doLogin(){doLoginWith(document.getElementById('pw').value);}
async function doLoginWith(pw){
  const cr=await(await fetch('/api/challenge')).json();
 const enc=new TextEncoder();
 const hash=await crypto.subtle.digest('SHA-256',enc.encode(pw));
 const hashArr=new Uint8Array(hash);
 const chBytes=new Uint8Array(atob(cr.challenge).split('').map(c=>c.charCodeAt(0)));
  const combined=new Uint8Array(hashArr.length+chBytes.length);
  combined.set(hashArr);combined.set(chBytes,hashArr.length);
  const resp=await crypto.subtle.digest('SHA-256',combined);
  const respHex=[...new Uint8Array(resp)].map(b=>b.toString(16).padStart(2,'0')).join('');
  const r=await fetch('/api/login',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({response:respHex})});
  if(r.ok){
   const d=await r.json();
   token=d.token||respHex;
   document.getElementById('recoveryStatus').textContent='';
   showMain();
  } else {alert('Login failed');}
 }
 async function armRecovery(){
  const r=await fetch('/api/recovery',{method:'POST'});
  if(!r||!r.ok){alert('Failed to arm recovery');return;}
  document.getElementById('recoveryStatus').textContent='Recovery armed for 30s. Toggle FSD on, then off, from the steering wheel.';
 }
async function refreshStats(){
  const r=await api('/api/status');if(!r)return;
  const d=await r.json();
  document.getElementById('handlerType').value=String(d.handler_type??1);
  document.getElementById('stats').innerHTML=
    `<div class='stat'><div class='val'>${d.rx_count||0}</div><div class='lbl'>RX</div></div>`
   +`<div class='stat'><div class='val'>${d.tx_count||0}</div><div class='lbl'>TX</div></div>`
   +`<div class='stat'><div class='val'>${d.tx_fail||0}</div><div class='lbl'>TX Fail</div></div>`
   +`<div class='stat'><div class='val'>${d.filtered||0}</div><div class='lbl'>Filtered</div></div>`
   +`<div class='stat'><div class='val'>${d.latency_us||'-'}</div><div class='lbl'>Latency us</div></div>`
    +`<div class='stat'><div class='val'>${d.fail_safe?'YES':'no'}</div><div class='lbl'>Fail Safe</div></div>`
    +`<div class='stat'><div class='val'>${d.recovery_armed?'ARMED':'idle'}</div><div class='lbl'>Recovery</div></div>`;
  document.getElementById('signals').innerHTML=
    `<div class='stat'><div class='val'>${d.handler_name||'-'}</div><div class='lbl'>Handler</div></div>`
    +`<div class='stat'><div class='val'>${d.fsd_selected?'YES':'no'}</div><div class='lbl'>FSD UI</div></div>`
    +`<div class='stat'><div class='val'>${fmtNum(d.follow_distance)}</div><div class='lbl'>Follow Dist</div></div>`
    +`<div class='stat'><div class='val'>${d.driving_side||'-'}</div><div class='lbl'>Driving Side</div></div>`
    +`<div class='stat'><div class='val'>${d.apmv3_branch||'-'}</div><div class='lbl'>APM Branch</div></div>`
    +`<div class='stat'><div class='val'>${fmtNum(d.autopilot_mode)}</div><div class='lbl'>AP Mode</div></div>`
    +`<div class='stat'><div class='val'>${fmtNum(d.speed_profile)}</div><div class='lbl'>Speed Profile</div></div>`
    +`<div class='stat'><div class='val'>${fmtNum(d.speed_offset)}</div><div class='lbl'>Speed Offset</div></div>`;
}
async function setHandler(){
 await api('/api/handler',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({type:parseInt(document.getElementById('handlerType').value)})});
}
async function refreshLog(){
 const r=await api('/api/can_log');if(!r)return;
 const d=await r.json();
 let txt=`Entries: ${d.count} Lost: ${d.lost}\n`;
 for(const e of (d.entries||[]).slice(-50)){
  txt+=`${e.ts} ${e.dir===1?'TX':'RX'} ${e.id.toString(16).padStart(3,'0')} [${e.dlc}] ${e.data.map(b=>b.toString(16).padStart(2,'0')).join(' ')}\n`;
 }
 document.getElementById('log').textContent=txt;
}
async function doOTA(){
 const f=document.getElementById('firmware').files[0];
 if(!f){alert('Select a file');return;}
 document.getElementById('otaStatus').textContent='Uploading...';
 const r=await api('/api/ota',{method:'POST',body:f});
 if(r&&r.ok)document.getElementById('otaStatus').textContent='Rebooting...';
 else document.getElementById('otaStatus').textContent='Upload failed';
}
init();
setInterval(()=>{if(token)refreshStats();},2000);
</script></body></html>)";

esp_err_t handlerIndex(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, kIndexHtml, sizeof(kIndexHtml) - 1);
    return ESP_OK;
}

esp_err_t handlerStatus(httpd_req_t* req) {
    can_fast::Stats st = can_fast::get_stats();
    int handler_type = can_fast::get_handler_type();
    SignalView view = buildSignalView();
    int follow_distance = view.has_driver_assist ? decodeFollowDistance(view.driver_assist) : -1;
    const char* driving_side = view.has_driver_assist ? drivingSideName(tesla_fsd::readDrivingSide(view.driver_assist)) : "unknown";
    bool fsd_selected = view.has_driver_assist && tesla_fsd::isFSDSelectedInUI(view.driver_assist);
    const char* apmv3_branch = view.has_mux1 ? branchName(tesla_fsd::readApmv3Branch(view.mux1)) : "unknown";
    int autopilot_mode = view.has_ap_mode ? decodeAutopilotMode(view.ap_mode) : -1;
    int speed_profile = decodeSpeedProfile(view, handler_type);
    int speed_offset = (handler_type == 1 && view.has_mux2) ? decodeSpeedOffset(view.mux2) : -1;

    char json[1024];
    int n = snprintf(json, sizeof(json),
        "{\"has_password\":%s,\"rx_count\":%lu,\"tx_count\":%lu,\"tx_fail\":%lu,"
        "\"filtered\":%lu,\"fail_safe\":%s,\"recovery_armed\":%s,\"latency_us\":%ld,"
        "\"max_latency_us\":%ld,\"min_latency_us\":%ld,\"handler_type\":%d,"
        "\"handler_name\":\"%s\",\"fsd_selected\":%s,\"follow_distance\":%d,"
        "\"driving_side\":\"%s\",\"apmv3_branch\":\"%s\","
        "\"autopilot_mode\":%d,\"speed_profile\":%d,\"speed_offset\":%d}",
        auth::has_password() ? "true" : "false",
        (unsigned long)st.rx_count, (unsigned long)st.tx_count,
        (unsigned long)st.tx_fail_count, (unsigned long)st.filtered_count,
        st.fail_safe ? "true" : "false",
        st.recovery_armed ? "true" : "false",
        (long)st.last_latency_us, (long)st.max_latency_us,
        (long)(st.min_latency_us == 0x7FFFFFFF ? 0 : st.min_latency_us),
        handler_type, handlerTypeName(handler_type),
        fsd_selected ? "true" : "false", follow_distance,
        driving_side, apmv3_branch,
        autopilot_mode, speed_profile, speed_offset);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, n);
    return ESP_OK;
}

esp_err_t handlerChallenge(httpd_req_t* req) {
    auth::ChallengeResponse cr = auth::get_challenge();
    char b64[32];
    size_t olen = 0;
    mbedtls_base64_encode(reinterpret_cast<unsigned char*>(b64), sizeof(b64), &olen,
                          cr.challenge, auth::kChallengeLen);
    b64[olen] = '\0';
    char json[128];
    int n = snprintf(json, sizeof(json), "{\"challenge\":\"%s\"}", b64);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, n);
    return ESP_OK;
}

bool parseHexResponse(const char* hex, size_t hexLen, uint8_t* out, size_t outLen) {
    if (hexLen != outLen * 2) return false;
    for (size_t i = 0; i < outLen; ++i) {
        unsigned int v;
        if (std::sscanf(hex + i * 2, "%02x", &v) != 1) return false;
        out[i] = static_cast<uint8_t>(v);
    }
    return true;
}

const char* findJsonString(const char* json, const char* key, char* val, size_t valLen, bool* truncated = nullptr) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    const char* p = std::strstr(json, pattern);
    if (!p) return nullptr;
    p += std::strlen(pattern);
    if (truncated) *truncated = false;
    size_t i = 0;
    while (*p && *p != '"' && i < valLen - 1) {
        val[i++] = *p++;
    }
    if (truncated && *p != '"') *truncated = true;
    val[i] = '\0';
    return val;
}

int findJsonInt(const char* json, const char* key) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    const char* p = std::strstr(json, pattern);
    if (!p) return -1;
    p += std::strlen(pattern);
    while (*p == ' ') p++;
    return std::atoi(p);
}

esp_err_t handlerLogin(httpd_req_t* req) {
    char buf[256] = {};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    char hexStr[128] = {};
    if (!findJsonString(buf, "response", hexStr, sizeof(hexStr))) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{}", 2);
        return ESP_FAIL;
    }
    uint8_t response[auth::kHashLen];
    if (!parseHexResponse(hexStr, std::strlen(hexStr), response, auth::kHashLen)) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{}", 2);
        return ESP_FAIL;
    }
    if (auth::verify_response(response, auth::kHashLen)) {
        char hex[auth::kHashLen * 2 + 1];
        for (size_t i = 0; i < auth::kHashLen; ++i)
            sprintf(hex + i * 2, "%02x", response[i]);
        hex[auth::kHashLen * 2] = '\0';
        char json[128];
        int n = snprintf(json, sizeof(json), "{\"token\":\"%s\"}", hex);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json, n);
        return ESP_OK;
    }
    httpd_resp_set_status(req, "403 Forbidden");
    httpd_resp_send(req, "{}", 2);
    return ESP_FAIL;
}

esp_err_t handlerSetup(httpd_req_t* req) {
    if (auth::has_password()) {
        httpd_resp_set_status(req, "403 Forbidden");
        httpd_resp_send(req, "{}", 2);
        return ESP_FAIL;
    }
    char buf[128] = {};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    char password[auth::kPasswordMaxLen + 1] = {};
    bool truncated = false;
    if (!findJsonString(buf, "password", password, sizeof(password), &truncated) || truncated) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{}", 2);
        return ESP_FAIL;
    }
    if (std::strlen(password) < 4) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{}", 2);
        return ESP_FAIL;
    }
    auth::set_password(password);
    httpd_resp_send(req, "{\"ok\":true}", 10);
    return ESP_OK;
}

esp_err_t handlerRecovery(httpd_req_t* req) {
    can_fast::arm_recovery();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"ok\":true,\"armed\":true}", 25);
    return ESP_OK;
}

esp_err_t handlerSetHandler(httpd_req_t* req) {
    if (!requireAuth(req)) return ESP_FAIL;
    char buf[64] = {};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    int type = findJsonInt(buf, "type");
    if (type < 0 || type > 2) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{}", 2);
        return ESP_FAIL;
    }
    can_fast::set_handler_type(type);
    httpd_resp_send(req, "{\"ok\":true}", 10);
    return ESP_OK;
}

esp_err_t handlerCanLog(httpd_req_t* req) {
    if (!requireAuth(req)) return ESP_FAIL;
    constexpr size_t kMaxEntries = 100;
    can_logger::Entry entries[kMaxEntries];
    size_t lost = 0;
    size_t count = can_logger::snapshot(entries, kMaxEntries, &lost);

    httpd_resp_set_type(req, "application/json");
    char hdr[128];
    int n = snprintf(hdr, sizeof(hdr),
        "{\"count\":%zu,\"lost\":%zu,\"entries\":[", count, lost);
    httpd_resp_send_chunk(req, hdr, n);

    for (size_t i = 0; i < count; ++i) {
        char entry[128];
        n = snprintf(entry, sizeof(entry),
            "%s{\"ts\":%u,\"id\":%u,\"dlc\":%u,\"dir\":%d,"
            "\"data\":[%d,%d,%d,%d,%d,%d,%d,%d]}",
            i > 0 ? "," : "",
            (unsigned)entries[i].timestamp_us,
            (unsigned)entries[i].frame.can_id,
            (unsigned)entries[i].frame.can_dlc,
            (int)entries[i].dir,
            entries[i].frame.data[0], entries[i].frame.data[1],
            entries[i].frame.data[2], entries[i].frame.data[3],
            entries[i].frame.data[4], entries[i].frame.data[5],
            entries[i].frame.data[6], entries[i].frame.data[7]);
        httpd_resp_send_chunk(req, entry, n);
    }
    httpd_resp_send_chunk(req, "]}", 2);
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}

esp_err_t handlerOTA(httpd_req_t* req) {
    if (!requireAuth(req)) return ESP_FAIL;
    if (!ota_update::begin()) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "{\"error\":\"ota_begin_failed\"}", 25);
        return ESP_FAIL;
    }
    char buf[4096];
    int received = 0;
    while ((received = httpd_req_recv(req, buf, sizeof(buf))) > 0) {
        if (!ota_update::write(reinterpret_cast<uint8_t*>(buf), received)) {
            ota_update::abort();
            httpd_resp_set_status(req, "500 Internal Server Error");
            httpd_resp_send(req, "{\"error\":\"write_failed\"}", 22);
            return ESP_FAIL;
        }
    }
    if (received < 0) {
        ota_update::abort();
        return ESP_FAIL;
    }
    if (!ota_update::end()) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "{\"error\":\"ota_finalize_failed\"}", 29);
        return ESP_FAIL;
    }
    httpd_resp_send(req, "{\"ok\":true}", 10);
    ota_update::schedule_reboot(500);
    return ESP_OK;
}

void registerHandlers() {
    httpd_uri_t uris[] = {
        {"/",           HTTP_GET,  handlerIndex,     nullptr},
        {"/api/status", HTTP_GET,  handlerStatus,    nullptr},
        {"/api/challenge",HTTP_GET,handlerChallenge,  nullptr},
        {"/api/login",  HTTP_POST, handlerLogin,     nullptr},
        {"/api/setup",  HTTP_POST, handlerSetup,     nullptr},
        {"/api/recovery",HTTP_POST, handlerRecovery,  nullptr},
        {"/api/handler",HTTP_POST, handlerSetHandler, nullptr},
        {"/api/can_log",HTTP_GET,  handlerCanLog,    nullptr},
        {"/api/ota",    HTTP_POST, handlerOTA,       nullptr},
    };
    for (auto& u : uris) {
        httpd_register_uri_handler(gServer, &u);
    }
}

}

void web_server_start() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 8192;
    config.max_uri_handlers = 12;
    if (httpd_start(&gServer, &config) == ESP_OK) {
        registerHandlers();
        ESP_LOGI(kLogTag, "Web server started on port %d", config.server_port);
    } else {
        ESP_LOGE(kLogTag, "Failed to start web server");
    }
}
