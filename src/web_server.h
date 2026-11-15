// HTTP server: web interface (web_ui.h) and JSON API.
//
// GET  /                     Web interface
// GET  /api/status           Network, MQTT and stove status with every metric
// POST /api/command          cmd=<MQTT command syntax> (ON, OFF, force off, T21, P3...)
// POST /api/poll             Read the stove now
// GET  /api/settings         Current settings (passwords are never returned)
// POST /api/settings/wifi    ssid, pass                         (restarts)
// POST /api/settings/mqtt    server, port, user, pass, topic    (restarts)
// POST /api/settings/general hostname, poll, hydro, webuser, webpass, noauth (restarts)
// GET  /api/wifi/scan        Nearby networks (call again while "scanning" is true)
// POST /api/metric           key, address, memory, divisor, enabled  (applied immediately)
// POST /api/metric/reset     key (or "all")
// POST /api/test/read        address, memory  -> one raw read
// POST /api/test/write       address, memory, value
// POST /api/test/readall     Reads every metric once and reports each result
// POST /api/test/scan        memory, start, end
// POST /api/test/onoff       seconds
// POST /api/test/stop
// GET  /api/test/status      Progress, scan results and log
// POST /api/reboot, /api/wifi/forget, /api/factory-reset
#pragma once

void webBegin();
void webLoop();
