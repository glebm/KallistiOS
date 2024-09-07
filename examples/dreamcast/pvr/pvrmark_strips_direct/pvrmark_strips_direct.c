/* KallistiOS ##version##

   pvrmark_strips_direct.c
   Copyright (C) 2002 Megan Potter
   Copyright (C) 2024 Falco Girgis
*/

/*
   This file serves as both an example of and benchmark for KOS's
   rendering fast path: the PVR direct rendering API. 
*/

#include <kos.h>
#include <stdlib.h>
#include <time.h>

pvr_init_params_t pvr_params = {
    { PVR_BINSIZE_16, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0, PVR_BINSIZE_0 },
    512 * 1024
};

enum { PHASE_HALVE, PHASE_INCR, PHASE_DECR, PHASE_FINAL };

static int polycnt;
static int phase = PHASE_HALVE;
static float avgfps = -1;
static pvr_poly_hdr_t hdr;
static time_t begin;

void running_stats(void) {
    pvr_stats_t stats;
    pvr_get_stats(&stats);

    if(avgfps == -1)
        avgfps = stats.frame_rate;
    else
        avgfps = (avgfps + stats.frame_rate) / 2.0f;
}

void stats(void) {
    pvr_stats_t stats;

    pvr_get_stats(&stats);
    dbglog(DBG_DEBUG, "3D Stats: %d frames, frame rate ~%f fps\n",
           stats.vbl_count, (double)stats.frame_rate);
}


int check_start(void) {
    maple_device_t *cont;
    cont_state_t *state;

    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    if(cont) {
        state = (cont_state_t *)maple_dev_status(cont);

        if(!state)
            return 0;

        return (state->buttons & CONT_START);
    }
    else
        return 0;
}

void setup(void) {
    pvr_poly_cxt_t cxt;

    pvr_init(&pvr_params);
    pvr_set_bg_color(0, 0, 0);

    pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
    cxt.gen.shading = PVR_SHADE_FLAT;
    pvr_poly_compile(&hdr, &cxt);
}


#define nextnum() seed = seed * 1164525 + 1013904223;
#define getnum(mn) (seed & ((mn) - 1))

void do_frame(void) {
    pvr_vertex_t * vert;
    int x, y, z;
    int i, col;
    static int oldseed = 0xdeadbeef;
    int seed = oldseed;
    pvr_dr_state_t dr_state;

    vid_border_color(0, 0, 0);
    pvr_wait_ready();
    vid_border_color(255, 0, 0);
    pvr_scene_begin();
    pvr_list_begin(PVR_LIST_OP_POLY);
    pvr_prim(&hdr, sizeof(hdr));

    pvr_dr_init(&dr_state);

    x = getnum(1024);
    nextnum();
    y = getnum(512);
    nextnum();
    z = getnum(128) + 1;
    nextnum();
    col = getnum(256);
    nextnum();

    vert = pvr_dr_target(dr_state);
    vert->flags = PVR_CMD_VERTEX;
    vert->x = x;
    vert->y = y;
    vert->z = z;
    vert->argb = col | (col << 8) | (col << 16) | 0xff000000;
    pvr_dr_commit(vert);

    for(i = 0; i < polycnt; i++) {
        x = (x + ((getnum(64)) - 32)) & 1023;
        nextnum();
        y = (y + ((getnum(64)) - 32)) % 511;
        nextnum();
        col = getnum(256);
        nextnum();
        vert = pvr_dr_target(dr_state);
        vert->flags = PVR_CMD_VERTEX;
        vert->x = x;
        vert->y = y;
        vert->z = z;
        vert->argb = col | (col << 8) | (col << 16) | 0xff000000;
        pvr_dr_commit(vert);
    }

    x = (x + ((getnum(64)) - 32)) & 1023;
    nextnum();
    y = (y + ((getnum(64)) - 32)) % 511;
    nextnum();
    col = getnum(256);
    nextnum();
    vert = pvr_dr_target(dr_state);
    vert->flags = PVR_CMD_VERTEX_EOL;
    vert->x = x;
    vert->y = y;
    vert->z = z;
    vert->argb = col | (col << 8) | (col << 16) | 0xff000000;
    pvr_dr_commit(vert);

    pvr_list_finish();
    pvr_scene_finish();
    vid_border_color(0, 255, 0);
    oldseed = seed;
}

void switch_tests(int ppf) {
    printf("Beginning new test: %d polys per frame (%d per second at 60fps)\n",
           ppf, ppf * 60);
    avgfps = -1;
    polycnt = ppf;
}

void check_switch(void) {
    time_t now;
    int new_polycnt = polycnt;

    now = time(NULL);

    if(now >= (begin + 5)) {
        printf("  Average Frame Rate: ~%f fps (%d pps)\n", (double)avgfps, (int)(polycnt * avgfps));

        switch(phase) {
            case PHASE_HALVE:

                if(avgfps < 55) {
                    new_polycnt = polycnt / 2;
                }
                else {
                    printf("  Entering PHASE_INCR\n");
                    phase = PHASE_INCR;
                    break;
                }

                break;
            case PHASE_INCR:

                if(avgfps >= 55) {
                    new_polycnt = polycnt + 5000;
                }
                else {
                    printf("  Entering PHASE_DECR\n");
                    phase = PHASE_DECR;
                    break;
                }

                break;
            case PHASE_DECR:

                if(avgfps < 55) {
                    new_polycnt = polycnt - 200;
                }
                else {
                    printf("  Entering PHASE_FINAL\n");
                    phase = PHASE_FINAL;
                    break;
                }

                break;
            case PHASE_FINAL:
                break;
        }

        begin = time(NULL);

        if(new_polycnt != polycnt)
            switch_tests(new_polycnt);
    }
}

int main(int argc, char **argv) {
    setup();

    /* Start off with something obscene */
    switch_tests(2000000 / 60);
    begin = time(NULL);

    for(;;) {
        if(check_start())
            break;

        printf(" \r");
        do_frame();
        running_stats();
        check_switch();
    }

    stats();

    return 0;
}


