//web_server.c

#include "web_server.h"
#include "data_model.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* DASHBOARD HTML */

static const char *dashboard_html =
"<!DOCTYPE html>"
"<html lang='en'>"
"<head>"
"  <meta charset='UTF-8'>"
"  <title>CAN Dashboard</title>"
"  <style>"

"    body{"
"      margin:0;"
"      font-family:Segoe UI,Arial,sans-serif;"
"      background:#f6f9fc;"
"      color:#0f172a;"
"    }"

"    header{"
"      padding:32px 20px;"
"      background:#ffffff;"
"      border-bottom:1px solid #e5e7eb;"
"      font-size:2.2rem;"
"      font-weight:800;"
"      text-align:center;"
"      letter-spacing:0.14em;"
"      text-transform:uppercase;"
"    }"

"    .dashboard{"
"      display:grid;"
"      grid-template-columns:repeat(auto-fit,minmax(180px,1fr));"
"      gap:24px;"
"      padding:40px;"
"      max-width:1100px;"
"      margin:auto;"
"    }"

"    .card{"
"      background:#ffffff;"
"      border-radius:16px;"
"      padding:24px 16px;"
"      text-align:center;"
"      border:1px solid #b0cef4;"
"      box-shadow:0 8px 20px rgba(0,0,0,0.08);"
"    }"

"    .icon{"
"      width:36px;"
"      height:36px;"
"      margin:0 auto 14px;"
"      stroke:#3b82f6;"
"      stroke-width:2;"
"      fill:none;"
"    }"

"    .label{"
"      font-size:0.75rem;"
"      letter-spacing:0.14em;"
"      color:#64748b;"
"      text-transform:uppercase;"
"      font-weight:600;"
"      margin-bottom:8px;"
"    }"

"    .value{"
"      font-size:2.6rem;"
"      font-weight:800;"
"      line-height:1;"
"    }"

"    .unit{"
"      font-size:0.85rem;"
"      color:#475569;"
"      margin-top:6px;"
"      font-weight:500;"
"    }"

"    .warn{"
"      margin-top:10px;"
"      font-size:0.8rem;"
"      font-weight:700;"
"      color:#dc2626;"
"    }"

"  </style>"
"</head>"

"<body>"

"<header>Vehicle Diagnostics</header>"

"<div class='dashboard'>"

/* Motor RPM */
"  <div class='card'>"
"    <svg class='icon' viewBox='0 0 24 24'>"
"      <circle cx='12' cy='12' r='9'></circle>"
"      <path d='M12 12 L18 6'></path>"
"    </svg>"
"    <div class='label'>Motor RPM</div>"
"    <div class='value'><span id='rpm'>---</span></div>"
"    <div class='unit'>rpm</div>"
"    <div id='rpm_warn' class='warn'></div>"
"  </div>"

/* Vehicle Speed */
"  <div class='card'>"
"    <svg class='icon' viewBox='0 0 24 24'>"
"      <path d='M4 14a8 8 0 0 1 16 0'></path>"
"      <circle cx='12' cy='14' r='1'></circle>"
"    </svg>"
"    <div class='label'>Vehicle Speed</div>"
"    <div class='value'><span id='speed'>---</span></div>"
"    <div class='unit'>km/h</div>"
"  </div>"

/* Battery Voltage */
"  <div class='card'>"
"    <svg class='icon' viewBox='0 0 24 24'>"
"      <rect x='2' y='7' width='18' height='10' rx='2'></rect>"
"      <line x1='22' y1='10' x2='22' y2='14'></line>"
"    </svg>"
"    <div class='label'>Battery Voltage</div>"
"    <div class='value'><span id='volt'>---</span></div>"
"    <div class='unit'>V</div>"
"  </div>"

/* Battery SOC */
"  <div class='card'>"
"    <svg class='icon' viewBox='0 0 24 24'>"
"      <rect x='2' y='7' width='18' height='10' rx='2'></rect>"
"      <line x1='22' y1='10' x2='22' y2='14'></line>"
"    </svg>"
"    <div class='label'>Battery SOC</div>"
"    <div class='value'><span id='soc'>---</span></div>"
"    <div class='unit'>%</div>"
"  </div>"

/* Motor Temperature */
"  <div class='card'>"
"    <svg class='icon' viewBox='0 0 24 24'>"
"      <path d='M10 14a4 4 0 1 0 4 0V5a2 2 0 0 0-4 0z'></path>"
"    </svg>"
"    <div class='label'>Motor Temperature</div>"
"    <div class='value'><span id='temp'>---</span></div>"
"    <div class='unit'>°C</div>"
"  </div>"

"</div>"

"<script>"
"async function update(){"
" const r = await fetch('/data');"
" const d = await r.json();"

" document.getElementById('rpm').innerText = d.motor_rpm;"
" document.getElementById('speed').innerText = d.vehicle_speed;"
" document.getElementById('soc').innerText = d.battery_soc;"
" document.getElementById('volt').innerText = d.battery_voltage;"
" document.getElementById('temp').innerText = d.motor_temperature;"

" if(d.rpm_warning)"
"  document.getElementById('rpm_warn').innerText='RPM OUT OF RANGE';"
" else"
"  document.getElementById('rpm_warn').innerText='';"
"}"
"setInterval(update,500);"
"update();"
"</script>"

"</body>"
"</html>";

/* SOCKET INCLUDES */

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#define PORT        8080
#define BUFFER_SIZE 1024

/* WEB SERVER ENTRY POINT */

void start_web_server(void)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    int server_fd;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket failed");
        return;
    }

    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port        = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("Bind failed");
        return;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        return;
    }

    printf("Web server running at http://localhost:%d\n", PORT);

    /* MAIN ACCEPT LOOP */

    while (1) {

#ifdef _WIN32
        int addrlen = sizeof(address);
        int client_fd =
            accept(server_fd, (struct sockaddr *)&address, &addrlen);
#else
        socklen_t addrlen = sizeof(address);
        int client_fd =
            accept(server_fd, (struct sockaddr *)&address, &addrlen);
#endif

        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);

#ifdef _WIN32
        recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
#else
        read(client_fd, buffer, BUFFER_SIZE - 1);
#endif

        /* ROOT PAGE ("/") */
        if (strncmp(buffer, "GET /", 5) == 0 &&
            strncmp(buffer, "GET /data", 9) != 0) {

            const char *header =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n\r\n";

#ifdef _WIN32
            send(client_fd, header, strlen(header), 0);
            send(client_fd, dashboard_html,
                 strlen(dashboard_html), 0);
            closesocket(client_fd);
#else
            write(client_fd, header, strlen(header));
            write(client_fd, dashboard_html,
                  strlen(dashboard_html));
            close(client_fd);
#endif
            continue;
        }

        /* DATA API ("/data") */
        if (strncmp(buffer, "GET /data", 9) == 0) {

            char response[512];
            snprintf(response, sizeof(response),
                     "HTTP/1.1 200 OK\r\n"
                     "Content-Type: application/json\r\n"
                     "Connection: close\r\n\r\n"
                     "{"
                     "\"motor_rpm\": %.2f,"
                     "\"rpm_warning\": %d,"
                     "\"vehicle_speed\": %.2f,"
                     "\"battery_soc\": %.2f,"
                     "\"battery_voltage\": %.2f,"
                     "\"motor_temperature\": %.2f"
                     "}",
                     g_vehicle_data.motor_rpm,
                     g_vehicle_data.rpm_warning,
                     g_vehicle_data.vehicle_speed,
                     g_vehicle_data.battery_soc,
                     g_vehicle_data.battery_voltage,
                     g_vehicle_data.motor_temperature);

#ifdef _WIN32
            send(client_fd, response, strlen(response), 0);
            closesocket(client_fd);
#else
            write(client_fd, response, strlen(response));
            close(client_fd);
#endif
            continue;
        }

        /* UNKNOWN PATH */
        const char *not_found =
            "HTTP/1.1 404 Not Found\r\n"
            "Connection: close\r\n\r\n";

#ifdef _WIN32
        send(client_fd, not_found, strlen(not_found), 0);
        closesocket(client_fd);
#else
        write(client_fd, not_found, strlen(not_found));
        close(client_fd);
#endif
    }
}
