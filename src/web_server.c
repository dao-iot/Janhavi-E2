#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "web_server.h"
#include "data_model.h"

/* DASHBOARD HTML */

static const char *dashboard_html =
"<!DOCTYPE html>"
"<html lang='en'>"
"<head>"
"<meta charset='UTF-8'>"
"<title>CAN Dashboard</title>"
"<style>"
"body{margin:0;font-family:Segoe UI,Arial;background:#f6f9fc;color:#0f172a;}"

"header{padding:32px;text-align:center;font-size:2.2rem;font-weight:800;"
"letter-spacing:0.14em;text-transform:uppercase;background:#fff;border-bottom:1px solid #656a73;}"

".dashboard{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));"
"gap:24px;padding:40px;max-width:1100px;margin:auto;}"

".card{background:#fff;border-radius:16px;padding:24px;text-align:center;"
"border:1px solid #618eca;box-shadow:0 8px 20px rgba(0,0,0,0.08);}"

".test-card{background:#fff;border-radius:16px;padding:24px;grid-column:span 5;"
"text-align:left;border:1px solid #618eca;box-shadow:0 8px 20px rgba(0,0,0,0.08);}"

/* ICONS */
".icon{font-size:28px;margin-bottom:10px;color:#2563eb;}"

".label{font-size:0.75rem;letter-spacing:0.14em;color:#64748b;"
"text-transform:uppercase;font-weight:600;margin-bottom:8px;}"

".value{font-size:2.6rem;font-weight:800;}"
".unit{font-size:0.85rem;color:#475569;margin-top:6px;}"
".warn{margin-top:8px;color:#dc2626;font-weight:700;font-size:0.8rem;}"
".test-entry{margin-top:10px;text-align:left;font-size:0.85rem;}"
"</style>"
"</head>"

"<body>"
"<header>Vehicle Diagnostics</header>"

"<div class='dashboard'>"

/* MOTOR RPM */
"<div class='card'>"
"<div class='icon'>⏱️</div>"
"<div class='label'>Motor RPM</div>"
"<div class='value'><span id='rpm'>---</span></div>"
"<div class='unit'>rpm</div>"
"<div id='rpm_warn' class='warn'></div>"
"</div>"

/* VEHICLE SPEED */
"<div class='card'>"
"<div class='icon'>🏎️</div>"
"<div class='label'>Vehicle Speed</div>"
"<div class='value'><span id='speed'>---</span></div>"
"<div class='unit'>km/h</div>"
"<div id='speed_warn' class='warn'></div>"
"</div>"

/* BATTERY VOLTAGE */
"<div class='card'>"
"<div class='icon'>⚡</div>"
"<div class='label'>Battery Voltage</div>"
"<div class='value'><span id='volt'>---</span></div>"
"<div class='unit'>V</div>"
"</div>"

/* BATTERY SOC */
"<div class='card'>"
"<div class='icon'>🔋</div>"
"<div class='label'>Battery SOC</div>"
"<div class='value'><span id='soc'>---</span></div>"
"<div class='unit'>%</div>"
"</div>"

/* MOTOR TEMPERATURE */
"<div class='card'>"
"<div class='icon'>🌡️</div>"
"<div class='label'>Motor Temperature</div>"
"<div class='value'><span id='temp'>---</span></div>"
"<div class='unit'>°C</div>"
"</div>"

/* TEST RESULTS */
"<div class='test-card'>"
"<div class='label'>Test Results</div>"
"<div id='tests'></div>"
"</div>"

"</div>"

"<script>"
"async function update(){"
"const r=await fetch('/data');"
"const d=await r.json();"

"if(d.mode===1){"
"let html='';"
"d.tests.forEach(function(t){"
"let color=(t.status===1)?'#16a34a':(t.status===2)?'#f59e0b':'#dc2626';"
"let label=(t.status===1)?'PASS':(t.status===2)?'WARNING':'ERROR';"
"html+=\"<div class='test-entry' style='color:\"+color+\"'>\";"
"html+=\"<b>\"+t.name+\": \"+label+\"</b><br>\";"
"html+=\"Input: \"+t.input+\"<br>Output: \"+t.output+\"</div>\";"
"});"
"document.getElementById('tests').innerHTML=html;"
"return;"
"}"

"document.getElementById('rpm').innerText=d.motor_rpm;"
"document.getElementById('speed').innerText=d.vehicle_speed;"
"document.getElementById('soc').innerText=d.battery_soc;"
"document.getElementById('volt').innerText=d.battery_voltage;"
"document.getElementById('temp').innerText=d.motor_temperature;"
"document.getElementById('rpm_warn').innerText=d.rpm_warning?'RPM OUT OF RANGE':'';"
"document.getElementById('speed_warn').innerText=d.speed_warning?'SPEED OUT OF RANGE':'';"
"}"
"setInterval(update,500);update();"
"</script>"

"</body></html>";


/* SOCKET INCLUDES */

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

/* WEB SERVER */

void start_web_server(void)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);
#endif

    int server_fd;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0){perror("socket");return;}

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    addr.sin_port=htons(PORT);

    if(bind(server_fd,(struct sockaddr*)&addr,sizeof(addr))<0){
        perror("bind");
        return;
    }

    listen(server_fd,3);
    printf("Web server running at http://localhost:%d\n",PORT);

    while(1){
        #ifdef _WIN32
            int len = sizeof(addr);
        #else
            socklen_t len = sizeof(addr);
        #endif
        int client = accept(server_fd,
                    (struct sockaddr *)&addr,
                    &len);
        if(client<0) continue;

        memset(buffer,0,BUFFER_SIZE);
#ifdef _WIN32
        recv(client,buffer,BUFFER_SIZE-1,0);
#else
        read(client,buffer,BUFFER_SIZE-1);
#endif

        /* ROOT */
        if(strncmp(buffer,"GET / ",6)==0){
            const char *hdr=
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n\r\n";
            send(client,hdr,strlen(hdr),0);
            send(client,dashboard_html,strlen(dashboard_html),0);
#ifdef _WIN32
            closesocket(client);
#else
            close(client);
#endif
            continue;
        }

        /* DATA */
        if(strncmp(buffer,"GET /data",9)==0){
            char resp[4096];

            if(g_vehicle_data.mode==1){
                int off=snprintf(resp,sizeof(resp),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Connection: close\r\n\r\n"
                    "{\"mode\":1,\"tests\":[");
                for(int i=0;i<g_vehicle_data.test_dashboard.count;i++){
                    TestResult *t=&g_vehicle_data.test_dashboard.results[i];
                    off+=snprintf(resp+off,sizeof(resp)-off,
                        "{\"name\":\"%s\",\"input\":\"%s\",\"output\":\"%s\",\"status\":%d}%s",
                        t->name,t->input,t->output,t->status,
                        (i<g_vehicle_data.test_dashboard.count-1)?",":"");
                }
                snprintf(resp+off,sizeof(resp)-off,"]}");
            } else {
                snprintf(resp,sizeof(resp),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Connection: close\r\n\r\n"
                    "{"
                    "\"mode\":2,"
                    "\"motor_rpm\":%.2f,"
                    "\"rpm_warning\":%d,"
                    "\"vehicle_speed\":%.2f,"
                    "\"speed_warning\":%d,"
                    "\"battery_soc\":%.2f,"
                    "\"battery_voltage\":%.2f,"
                    "\"motor_temperature\":%.2f"
                    "}",
                    g_vehicle_data.motor_rpm,
                    g_vehicle_data.rpm_warning,
                    g_vehicle_data.vehicle_speed,
                    g_vehicle_data.speed_warning,
                    g_vehicle_data.battery_soc,
                    g_vehicle_data.battery_voltage,
                    g_vehicle_data.motor_temperature);
            }

            send(client,resp,strlen(resp),0);
#ifdef _WIN32
            closesocket(client);
#else
            close(client);
#endif
        }
    }
}