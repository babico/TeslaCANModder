import { buildApplyTransportMessage } from "../../src/hardware/monitorTransportMessages";
import { getMonitorTransportOption } from "../../src/hardware/transportPresentation";

describe("monitorTransportMessages", () => {
  it("returns REST update message for http transport", () => {
    const option = getMonitorTransportOption("http");
    expect(
      buildApplyTransportMessage("http", option, "http://192.168.4.1", "/api/command")
    ).toBe("REST transport updated: http://192.168.4.1/api/command");
  });

  it("returns adapter guidance for non-http transport", () => {
    const option = getMonitorTransportOption("ble");
    expect(
      buildApplyTransportMessage("ble", option, "http://192.168.4.1", "/api/command")
    ).toBe(
      "BLE selected. Connect a runtime adapter with connectViaBle(...) to activate this monitor transport."
    );
  });
});
