# Tesla-drug fork catalog (2026-11-20)

Research-only inventory for `babico/TeslaCANModder` `legacy/`. **Do not run against an actual vehicle or treat these sources as a verified FSD unlock.** The reported feature-unlock behavior is unverified and can affect safety-critical driver-assistance behavior. No files are integrated into shipping firmware, client, or protocol code.

- Search: GitHub repository search `Tesla-drug fork:true` (pages 1–2, up to 100 results per page).
- Results indexed: **152** accessible public repositories (a search-time snapshot; not a complete audit of all GitHub forks).
- Original: [zeratulhx/Tesla-drug](https://github.com/zeratulhx/Tesla-drug) returned HTTP 404 on 2026-11-20. A 404 does not establish whether it was deleted or made private.
- Strategy (updated 2026-11-23): the full source list below is retained, and **149 repositories are pinned as read-only submodules** under `legacy/<owner>-<repo>`. The four originally pinned reference snapshots are listed in the table below. The remaining 3 entries are catalog-only because upstream returns HTTP 404 (`Aihuange/Tesla-drug`, `GXJ0102/Tesla-drug`, `yangyangha1/Tesla-drug` — re-verified 2026-11-23). Earlier catalog sizes of `0` were a GitHub search-snapshot artifact, not empty repos: the 10 such entries were added on 2026-11-23 and all point at the same commit as `cos003/tesla-drug` (`7def0dcc`). CI checks out with `submodules: false`, so the extra gitlinks do not clone on every run.
- GitHub reported repository size is not proof of content equality. Not every entry below was independently byte-compared. Additional revisions may exist.
- The ZIP/certificate/key material in forks is untrusted and has **not** been executed or validated. Review licenses and provenance before redistribution or use.

## Pinned reference snapshots

| Submodule under `legacy/` | Source | Commit | Notes |
| --- | --- | --- | --- |
| [`legacy/jdgbv2-Tesla-drug`](../../legacy/jdgbv2-Tesla-drug) | [jdgbv2/Tesla-drug](https://github.com/jdgbv2/Tesla-drug) | [`05d2b265a372`](https://github.com/jdgbv2/Tesla-drug/commit/05d2b265a372d43f2eaf0e00bf7ce8be7d5c05d8) | PowerShell/Python intercept scripts; original installer variant |
| [`legacy/Arispex-Tesla-drug`](../../legacy/Arispex-Tesla-drug) | [Arispex/Tesla-drug](https://github.com/Arispex/Tesla-drug) | [`ca02db4f831e`](https://github.com/Arispex/Tesla-drug/commit/ca02db4f831ec4fdb148e57684c1dede050ffa45) | Different PowerShell script revisions |
| [`legacy/cos003-tesla-drug`](../../legacy/cos003-tesla-drug) | [cos003/tesla-drug](https://github.com/cos003/tesla-drug) | [`7def0dcc9ed0`](https://github.com/cos003/tesla-drug/commit/7def0dcc9ed0e91a5476f82487ecaefe223d20bd) | README, LICENSE and CLEAN-2026.zip (uninspected archive) |
| [`legacy/ThinkHao-Tesla-drug`](../../legacy/ThinkHao-Tesla-drug) | [ThinkHao/Tesla-drug](https://github.com/ThinkHao/Tesla-drug) | [`274d4e812ea2`](https://github.com/ThinkHao/Tesla-drug/commit/274d4e812ea2245feceaff149471214a51ce50e5) | Expanded cross-platform files, scripts and fsd-mitm |

The four commits were verified from their respective `main` branch metadata; the copies were not checked out or executed. Use `git submodule update --init <path>` only for isolated offline research with a separately assessed trust boundary.

## All discovered repository links

The following is the source inventory. Entries marked `pinned` are the four originally selected reference snapshots; all other reachable entries were subsequently added as submodules as well (including those whose catalog size showed `0`). The three 404 upstreams remain catalog-only.

| # | Repository | GitHub size (KiB) | Pin |
| ---: | --- | ---: | :---: |
| 1 | [0xtulaoda/tesla-drug](https://github.com/0xtulaoda/tesla-drug) | 85 | — |
| 2 | [26CSNEWBIE/Tesla-drug](https://github.com/26CSNEWBIE/Tesla-drug) | 235 | — |
| 3 | [795795/Tesla-drug](https://github.com/795795/Tesla-drug) | 85 | — |
| 4 | [79594593/Tesla-drug](https://github.com/79594593/Tesla-drug) | 75 | — |
| 5 | [96syh/Tesla-drug](https://github.com/96syh/Tesla-drug) | 85 | — |
| 6 | [a869269559-png/Tesla-drug](https://github.com/a869269559-png/Tesla-drug) | 75 | — |
| 7 | [aibertmark/Tesla-drug](https://github.com/aibertmark/Tesla-drug) | 75 | — |
| 8 | [Aihuange/Tesla-drug](https://github.com/Aihuange/Tesla-drug) | 85 | — |
| 9 | [alansdragon0/Tesla-drug-backup](https://github.com/alansdragon0/Tesla-drug-backup) | 85 | — |
| 10 | [Aresonlyone/tesla-drug](https://github.com/Aresonlyone/tesla-drug) | 85 | — |
| 11 | [Arispex/Tesla-drug](https://github.com/Arispex/Tesla-drug) | 75 | pinned |
| 12 | [AxnCode/Tesla-drug](https://github.com/AxnCode/Tesla-drug) | 75 | — |
| 13 | [bfdream/Tesla-drug](https://github.com/bfdream/Tesla-drug) | 85 | — |
| 14 | [bi4nbn/Tesla-drug](https://github.com/bi4nbn/Tesla-drug) | 75 | — |
| 15 | [Bu-TianLie/Tesla-drug](https://github.com/Bu-TianLie/Tesla-drug) | 85 | — |
| 16 | [caijianhai/Tesla-drug](https://github.com/caijianhai/Tesla-drug) | 85 | — |
| 17 | [Caun1112/Tesla-drug](https://github.com/Caun1112/Tesla-drug) | 85 | — |
| 18 | [cavinding64-hub/Tesla-drug](https://github.com/cavinding64-hub/Tesla-drug) | 75 | — |
| 19 | [cccyun1/Tesla-drug](https://github.com/cccyun1/Tesla-drug) | 75 | — |
| 20 | [chinawpm/Tesla-drug](https://github.com/chinawpm/Tesla-drug) | 75 | — |
| 21 | [ciaizmai1024-sudo/Tesla-drug](https://github.com/ciaizmai1024-sudo/Tesla-drug) | 85 | — |
| 22 | [cker321/Tesla-drug](https://github.com/cker321/Tesla-drug) | 0 | — |
| 23 | [Coolkeke/Tesla-drug](https://github.com/Coolkeke/Tesla-drug) | 75 | — |
| 24 | [cos003/tesla-drug](https://github.com/cos003/tesla-drug) | 149 | pinned |
| 25 | [crceer/Tesla-drug](https://github.com/crceer/Tesla-drug) | 75 | — |
| 26 | [ct3987175-lgtm/Tesla-drug](https://github.com/ct3987175-lgtm/Tesla-drug) | 85 | — |
| 27 | [cutebearowner/Tesla-drug](https://github.com/cutebearowner/Tesla-drug) | 85 | — |
| 28 | [DennisYa76/Tesla-drug](https://github.com/DennisYa76/Tesla-drug) | 85 | — |
| 29 | [donadoo/Tesla-drug](https://github.com/donadoo/Tesla-drug) | 85 | — |
| 30 | [ehzuf/Tesla-drug](https://github.com/ehzuf/Tesla-drug) | 85 | — |
| 31 | [etenhele/Tesla-drug](https://github.com/etenhele/Tesla-drug) | 75 | — |
| 32 | [Fan-sir/Tesla-drug](https://github.com/Fan-sir/Tesla-drug) | 85 | — |
| 33 | [FandelXiong/Tesla-drug](https://github.com/FandelXiong/Tesla-drug) | 85 | — |
| 34 | [fbion/Tesla-drug](https://github.com/fbion/Tesla-drug) | 85 | — |
| 35 | [fengchen320/Tesla-drug](https://github.com/fengchen320/Tesla-drug) | 149 | — |
| 36 | [FishNaCl/Tesla-drug](https://github.com/FishNaCl/Tesla-drug) | 85 | — |
| 37 | [FlowerMenaiZi/Tesla-drug](https://github.com/FlowerMenaiZi/Tesla-drug) | 85 | — |
| 38 | [flytam/Tesla-drug](https://github.com/flytam/Tesla-drug) | 0 | — |
| 39 | [FT-Fetters/Tesla-drug](https://github.com/FT-Fetters/Tesla-drug) | 75 | — |
| 40 | [FZ997616100/Tesla-drug](https://github.com/FZ997616100/Tesla-drug) | 0 | — |
| 41 | [g-ych/Tesla-drug](https://github.com/g-ych/Tesla-drug) | 85 | — |
| 42 | [gba3455/Tesla-drug](https://github.com/gba3455/Tesla-drug) | 0 | — |
| 43 | [GXJ0102/Tesla-drug](https://github.com/GXJ0102/Tesla-drug) | 75 | — |
| 44 | [HanXing1694/Tesla-drug](https://github.com/HanXing1694/Tesla-drug) | 85 | — |
| 45 | [hddujianlong-ui/Tesla-drug](https://github.com/hddujianlong-ui/Tesla-drug) | 75 | — |
| 46 | [huangfengye/Tesla-drug](https://github.com/huangfengye/Tesla-drug) | 85 | — |
| 47 | [huyijie04/Tesla-drug](https://github.com/huyijie04/Tesla-drug) | 75 | — |
| 48 | [i520e3e4/Tesla-drug](https://github.com/i520e3e4/Tesla-drug) | 85 | — |
| 49 | [iampennyli/Tesla-drug](https://github.com/iampennyli/Tesla-drug) | 85 | — |
| 50 | [Ikepi/Tesla-drug](https://github.com/Ikepi/Tesla-drug) | 75 | — |
| 51 | [JamisonMurphy/Tesla-drug](https://github.com/JamisonMurphy/Tesla-drug) | 75 | — |
| 52 | [jasonkarl/Tesla-drug](https://github.com/jasonkarl/Tesla-drug) | 149 | — |
| 53 | [Jclevy-CN/Tesla-drug](https://github.com/Jclevy-CN/Tesla-drug) | 75 | — |
| 54 | [jdgbv2/Tesla-drug](https://github.com/jdgbv2/Tesla-drug) | 75 | pinned |
| 55 | [jiahao2333/Tesla-drug](https://github.com/jiahao2333/Tesla-drug) | 75 | — |
| 56 | [JiaQ14/tesla-drug](https://github.com/JiaQ14/tesla-drug) | 75 | — |
| 57 | [jjjfff08/Tesla-drug](https://github.com/jjjfff08/Tesla-drug) | 75 | — |
| 58 | [jnxbk/Tesla-drug](https://github.com/jnxbk/Tesla-drug) | 85 | — |
| 59 | [jt882094-spec/Tesla-drug](https://github.com/jt882094-spec/Tesla-drug) | 85 | — |
| 60 | [Kaide0521/Tesla-drug](https://github.com/Kaide0521/Tesla-drug) | 75 | — |
| 61 | [Kaiser-DMr/Tesla-drug](https://github.com/Kaiser-DMr/Tesla-drug) | 85 | — |
| 62 | [ken861222/Tesla-drug](https://github.com/ken861222/Tesla-drug) | 75 | — |
| 63 | [kenta-do/Tesla-drug](https://github.com/kenta-do/Tesla-drug) | 85 | — |
| 64 | [kerasqun/Tesla-drug](https://github.com/kerasqun/Tesla-drug) | 85 | — |
| 65 | [kothedog/Tesla-drug](https://github.com/kothedog/Tesla-drug) | 85 | — |
| 66 | [kyriewxcode/Tesla-drug](https://github.com/kyriewxcode/Tesla-drug) | 0 | — |
| 67 | [l690604972-art/tesla-drug](https://github.com/l690604972-art/tesla-drug) | 75 | — |
| 68 | [la-cctv/tesla-drug](https://github.com/la-cctv/tesla-drug) | 149 | — |
| 69 | [leakeung/Tesla-drug](https://github.com/leakeung/Tesla-drug) | 85 | — |
| 70 | [leftjs/Tesla-drug](https://github.com/leftjs/Tesla-drug) | 85 | — |
| 71 | [lenfien/Tesla-drug](https://github.com/lenfien/Tesla-drug) | 0 | — |
| 72 | [lgp2013/Tesla-drug](https://github.com/lgp2013/Tesla-drug) | 85 | — |
| 73 | [liangfb/Tesla-drug](https://github.com/liangfb/Tesla-drug) | 85 | — |
| 74 | [liangts/tesla-drug](https://github.com/liangts/tesla-drug) | 85 | — |
| 75 | [LitePenguins/Tesla-drug](https://github.com/LitePenguins/Tesla-drug) | 85 | — |
| 76 | [liuxiaowoy/Tesla-drug](https://github.com/liuxiaowoy/Tesla-drug) | 85 | — |
| 77 | [lockys/Tesla-drug](https://github.com/lockys/Tesla-drug) | 85 | — |
| 78 | [LouisTigerKid/Tesla-drug](https://github.com/LouisTigerKid/Tesla-drug) | 75 | — |
| 79 | [lovelesslove/Tesla-drug](https://github.com/lovelesslove/Tesla-drug) | 149 | — |
| 80 | [lxyphotography-web/Tesla-drug](https://github.com/lxyphotography-web/Tesla-drug) | 75 | — |
| 81 | [mambablack/Tesla-drug](https://github.com/mambablack/Tesla-drug) | 85 | — |
| 82 | [Mansorry101/Tesla-drug](https://github.com/Mansorry101/Tesla-drug) | 85 | — |
| 83 | [MeowRay/Tesla-drug](https://github.com/MeowRay/Tesla-drug) | 149 | — |
| 84 | [mishchael/Tesla-drug](https://github.com/mishchael/Tesla-drug) | 85 | — |
| 85 | [mitcehub/Tesla-drug](https://github.com/mitcehub/Tesla-drug) | 75 | — |
| 86 | [MrHongping/Tesla-drug](https://github.com/MrHongping/Tesla-drug) | 85 | — |
| 87 | [N0TH1N9cyber/Tesla-drug](https://github.com/N0TH1N9cyber/Tesla-drug) | 85 | — |
| 88 | [natsukiking/Tesla-drug](https://github.com/natsukiking/Tesla-drug) | 85 | — |
| 89 | [Neror1/Tesla-drug](https://github.com/Neror1/Tesla-drug) | 85 | — |
| 90 | [OneIFoR/Tesla-drug](https://github.com/OneIFoR/Tesla-drug) | 85 | — |
| 91 | [onemiless/Tesla-drug](https://github.com/onemiless/Tesla-drug) | 75 | — |
| 92 | [pandan129-pixel/tesla-drug](https://github.com/pandan129-pixel/tesla-drug) | 75 | — |
| 93 | [PennyVibe/Tesla-drug](https://github.com/PennyVibe/Tesla-drug) | 75 | — |
| 94 | [picasiso/Tesla-drug](https://github.com/picasiso/Tesla-drug) | 75 | — |
| 95 | [pizigao/Tesla-drug](https://github.com/pizigao/Tesla-drug) | 0 | — |
| 96 | [porkmanic/Tesla-drug](https://github.com/porkmanic/Tesla-drug) | 85 | — |
| 97 | [qingzhi0508/Tesla-drug](https://github.com/qingzhi0508/Tesla-drug) | 85 | — |
| 98 | [readpan/Tesla-drug](https://github.com/readpan/Tesla-drug) | 85 | — |
| 99 | [royan0596/Tesla-drug](https://github.com/royan0596/Tesla-drug) | 75 | — |
| 100 | [samDDD123/Tesla-drug-sam](https://github.com/samDDD123/Tesla-drug-sam) | 85 | — |
| 101 | [sdddhhll/Tesla-drug](https://github.com/sdddhhll/Tesla-drug) | 85 | — |
| 102 | [shawlp/Tesla-drug](https://github.com/shawlp/Tesla-drug) | 85 | — |
| 103 | [shm197/tesla-drug](https://github.com/shm197/tesla-drug) | 75 | — |
| 104 | [SillyRoot/Tesla-drug](https://github.com/SillyRoot/Tesla-drug) | 85 | — |
| 105 | [SimonRen/Tesla-drug](https://github.com/SimonRen/Tesla-drug) | 85 | — |
| 106 | [skyjerry/Tesla-drug](https://github.com/skyjerry/Tesla-drug) | 75 | — |
| 107 | [smockgithub/Tesla-drug](https://github.com/smockgithub/Tesla-drug) | 85 | — |
| 108 | [sonicmingit/Tesla-drug](https://github.com/sonicmingit/Tesla-drug) | 85 | — |
| 109 | [suanwork-cmd/Tesla-drug](https://github.com/suanwork-cmd/Tesla-drug) | 85 | — |
| 110 | [sunruijie1129/Tesla-drug](https://github.com/sunruijie1129/Tesla-drug) | 85 | — |
| 111 | [SupXL/Tesla-drug](https://github.com/SupXL/Tesla-drug) | 85 | — |
| 112 | [syjjx/Tesla-drug](https://github.com/syjjx/Tesla-drug) | 75 | — |
| 113 | [ThinkHao/Tesla-drug](https://github.com/ThinkHao/Tesla-drug) | 166 | pinned |
| 114 | [TianzeWang/Tesla-drug](https://github.com/TianzeWang/Tesla-drug) | 75 | — |
| 115 | [TommyLee96/Tesla-drug](https://github.com/TommyLee96/Tesla-drug) | 149 | — |
| 116 | [topiman/Tesla-drug](https://github.com/topiman/Tesla-drug) | 85 | — |
| 117 | [try-to-fly/Tesla-drug](https://github.com/try-to-fly/Tesla-drug) | 85 | — |
| 118 | [TT702/Tesla-drug](https://github.com/TT702/Tesla-drug) | 149 | — |
| 119 | [tttggg123/Tesla-drug](https://github.com/tttggg123/Tesla-drug) | 75 | — |
| 120 | [tulaoda/tesla-drug](https://github.com/tulaoda/tesla-drug) | 0 | — |
| 121 | [TW100x/Tesla-drug](https://github.com/TW100x/Tesla-drug) | 85 | — |
| 122 | [tyk2018/Tesla-drug](https://github.com/tyk2018/Tesla-drug) | 75 | — |
| 123 | [upseem/Tesla-drug](https://github.com/upseem/Tesla-drug) | 85 | — |
| 124 | [uu-myheart/Tesla-drug](https://github.com/uu-myheart/Tesla-drug) | 85 | — |
| 125 | [vaakian/tesla-drug](https://github.com/vaakian/tesla-drug) | 149 | — |
| 126 | [vangohao/Tesla-drug](https://github.com/vangohao/Tesla-drug) | 75 | — |
| 127 | [vizstorylab/Tesla-drug](https://github.com/vizstorylab/Tesla-drug) | 75 | — |
| 128 | [Weilakuba/Tesla-drug](https://github.com/Weilakuba/Tesla-drug) | 85 | — |
| 129 | [wlrnet/Tesla-drug](https://github.com/wlrnet/Tesla-drug) | 75 | — |
| 130 | [wujianyin/Tesla-drug](https://github.com/wujianyin/Tesla-drug) | 75 | — |
| 131 | [x0dm/Tesla-drug](https://github.com/x0dm/Tesla-drug) | 85 | — |
| 132 | [x1aner/Tesla-drug](https://github.com/x1aner/Tesla-drug) | 85 | — |
| 133 | [xfbbb/Tesla-drug](https://github.com/xfbbb/Tesla-drug) | 85 | — |
| 134 | [Xiapiiiii/Tesla-drug](https://github.com/Xiapiiiii/Tesla-drug) | 85 | — |
| 135 | [xisungod/Tesla-drug](https://github.com/xisungod/Tesla-drug) | 0 | — |
| 136 | [xxxxxyyyy/Tesla-drug](https://github.com/xxxxxyyyy/Tesla-drug) | 85 | — |
| 137 | [xyzabcmao/Tesla-drug](https://github.com/xyzabcmao/Tesla-drug) | 75 | — |
| 138 | [yakejkl/Tesla-drug](https://github.com/yakejkl/Tesla-drug) | 85 | — |
| 139 | [yan01yan18/Tesla-drug](https://github.com/yan01yan18/Tesla-drug) | 85 | — |
| 140 | [yandlee/Tesla-drug](https://github.com/yandlee/Tesla-drug) | 85 | — |
| 141 | [yangbb84/Tesla-drug](https://github.com/yangbb84/Tesla-drug) | 0 | — |
| 142 | [yangyangha1/Tesla-drug](https://github.com/yangyangha1/Tesla-drug) | 85 | — |
| 143 | [yecunbonbo/Tesla-drug](https://github.com/yecunbonbo/Tesla-drug) | 149 | — |
| 144 | [YishunHa/Tesla-drug](https://github.com/YishunHa/Tesla-drug) | 85 | — |
| 145 | [ytc214800722/Tesla-drug](https://github.com/ytc214800722/Tesla-drug) | 85 | — |
| 146 | [ytwddtt/Tesla-drug](https://github.com/ytwddtt/Tesla-drug) | 149 | — |
| 147 | [Yxiaoyu/Tesla-drug](https://github.com/Yxiaoyu/Tesla-drug) | 85 | — |
| 148 | [zeroall88/Tesla-drug](https://github.com/zeroall88/Tesla-drug) | 85 | — |
| 149 | [zf9097/Tesla-drug](https://github.com/zf9097/Tesla-drug) | 75 | — |
| 150 | [zjuhjz/Tesla-drug](https://github.com/zjuhjz/Tesla-drug) | 85 | — |
| 151 | [zkcoc/Tesla-drug](https://github.com/zkcoc/Tesla-drug) | 85 | — |
| 152 | [zxerqw/Tesla-drug](https://github.com/zxerqw/Tesla-drug) | 85 | — |

Metadata is a point-in-time listing. Recheck source availability and license before updating pins.
