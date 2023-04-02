// Originally from plib_examples
// Ported to Dreamcast/KOS by Peter Hatch
// read_input () code from KOS examples
// Converted to a clock by Megan Potter
// Timer functionality added by Falco Girgis

#include <kos.h>
#include <ctime>
#include <dcplib/fnt.h>
#include <chrono>
#include <sstream>
#include <string>
#include <sys/time.h>

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

fntRenderer *text;
fntTexFont *font;

int filter_mode = 0;

const char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                         "Aug", "Sep", "Oct", "Nov", "Dec"
                       };

float bg[3];                /* Current bg */
float bg_delta[3] = { 0.01f };      /* bg per-frame delta */
int   bg_cur = 1;
float bgarray[][3] = {
    { 0.0f, 0.0f, 0.0f },
    { 0.5f, 0.0f, 0.0f },
    { 0.0f, 0.5f, 0.0f },
    { 0.0f, 0.0f, 0.5f },
    { 0.5f, 0.0f, 0.5f },
    { 0.0f, 0.5f, 0.5f },
    { 0.5f, 0.5f, 0.0f },
    { 0.5f, 0.5f, 0.5f }
};
#define BG_COUNT 8

#define fabs(a) ( (a) < 0 ? -(a) : (a) )
void bgframe() {
    pvr_set_bg_color(bg[0], bg[1], bg[2]);

    bg[0] += bg_delta[0];
    bg[1] += bg_delta[1];
    bg[2] += bg_delta[2];

    if(fabs(bg[0] - bgarray[bg_cur][0]) < 0.01f
            && fabs(bg[1] - bgarray[bg_cur][1]) < 0.01f
            && fabs(bg[2] - bgarray[bg_cur][2]) < 0.01f) {
        bg_cur++;

        if(bg_cur >= BG_COUNT)
            bg_cur = 0;

        bg_delta[0] = (bgarray[bg_cur][0] - bg[0]) / (0.5f / 0.01f);
        bg_delta[1] = (bgarray[bg_cur][1] - bg[1]) / (0.5f / 0.01f);
        bg_delta[2] = (bgarray[bg_cur][2] - bg[2]) / (0.5f / 0.01f);
    }
}

void drawFrame() {
    char tmpbuf[256];
    int y = 50;

    bgframe();

    struct timespec spec;
    timespec_get(&spec, TIME_UTC);
    struct tm *brokenDown = localtime(&spec.tv_sec);

    pvr_wait_ready();
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_TR_POLY);

    text->setFilterMode(filter_mode);

    text->setFont(font);
    text->setPointSize(30);

    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts("(Not So) Simple DC Clock");
    text->end();
    y += 50;

    sprintf(tmpbuf, "%s %s %02u %04u",
            days[brokenDown->tm_wday], 
            months[brokenDown->tm_mon], 
            brokenDown->tm_mday, 
            1900 + brokenDown->tm_year);

    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts(tmpbuf);
    text->end();
    y += 50;

    sprintf(tmpbuf, "Unix Time: %llu.%.9lu", spec.tv_sec, spec.tv_nsec);

    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts(tmpbuf);
    text->end();
    y += 50;

    sprintf(tmpbuf, "C Time: %2u:%02u:%02u.%.9lu",
            brokenDown->tm_hour, brokenDown->tm_min, brokenDown->tm_sec, spec.tv_nsec);

    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts(tmpbuf);
    text->end();
    y += 50;

    std::stringstream ss;

    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();

    typedef std::chrono::duration<int> Days; /* UTC: +0:00 */

    Days days = std::chrono::duration_cast<Days>(duration);
        duration -= days;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
        duration -= hours;
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration);
        duration -= minutes;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
        duration -= seconds;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        duration -= milliseconds;
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
        duration -= microseconds;
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

    std::time_t nowTime = std::chrono::high_resolution_clock::to_time_t(now);

    ss << std::ctime(&nowTime);
#if 0
    ss        << "C++ Chrono: " 
              << hours.count() << ":"
              << minutes.count() << ":"
              << seconds.count() << ":"
              << milliseconds.count() << ":"
              << microseconds.count() << ":"
              << nanoseconds.count() << std::endl;
#endif
    std::string cppStr = ss.str();
    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts(cppStr.c_str());
    text->end();
    y += 50;

    char buffer[100];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    brokenDown = localtime(&tv.tv_sec);
    strftime(buffer, sizeof(buffer), "%I:%M:%S %p", brokenDown);

    ss.str("");

    ss << "POSIX Time: " << buffer; 

    cppStr = ss.str();
    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts(cppStr.c_str());
    text->end();
    y += 50;

    const clock_t clockValue = clock();
    const unsigned clockSecs = clockValue / CLOCKS_PER_SEC;
    const unsigned clockUSecs = clockValue % CLOCKS_PER_SEC;
    sprintf(tmpbuf, "C clock: %u.%.6u", clockSecs, clockUSecs);
    text->begin();
    text->setColor(1, 1, 1);
    text->start2f(20, y);
    text->puts(tmpbuf);
    text->end();
    y += 50;


    pvr_list_finish();
    pvr_scene_finish();
}


/* This is really more complicated than it needs to be in this particular
   case, but it's nice and verbose. */
int read_input() {
    maple_device_t *cont;
    cont_state_t *state;

    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    if(!cont) {
        return 0;
    }

    /* Check for start on the controller */
    state = (cont_state_t *)maple_dev_status(cont);

    if(!state) {
        printf("Error getting controller status\n");
        return 1;
    }

    if(state->buttons & CONT_START) {
        printf("Pressed start\n");
        return 1;
    }

    return 0;
}

int main(int argc, char **argv) {
    pvr_init_params_t pvrInit = { {PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_32, PVR_BINSIZE_0, PVR_BINSIZE_0}, 512 * 1024};
    pvr_init(&pvrInit);

    text = new fntRenderer();
    font = new fntTexFont("/rd/default.txf");

    while(!read_input()) {
        drawFrame();
    }

    return 0;
}
