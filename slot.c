/*
 *  slot machine analyzer
 *
 *  1 coin, 1 play
 *
 *  copyright (c) 2010, 2011
 *  georgeacraig@gmail.com. all rights reserved.
 *
 *  (1) thanks mike fagan
 *  (2) seed() cp'ed from julienne walker
 *      http://www.eternallyconfuzzled.com/arts/jsw_art_rand.aspx
 *  (3) incredible lego slot machine
 *      http://www.youtube.com/watch?v=ET1DoB5sKpo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <locale.h>

#define REEL_STOPS       64
#define NUM_REELS        3

#define SYM_SPACE        1
#define SYM_BLUE_SEVEN   2
#define SYM_WHITE_BAR    3
#define SYM_RED_BAR      4
#define SYM_BLUE_BAR     5
#define SYM_WHITE_SEVEN  6
#define SYM_RED_SEVEN    7

int any_three_spaces();
int any_three_blue();
int any_three_white();
int any_three_red();
int any_three_bars();
int all_three_red_bars();
int any_redwhiteblue();
int all_three_white_bars();
int all_three_blue_bars();
int red_white_blue_bars();
int any_three_sevens();
int all_three_blue_sevens();
int all_three_white_sevens();
int all_three_red_sevens();
int red_white_blue_sevens();

long total_hits, total_pays;
float hit_per, payback_per, hold_per;

struct slot_machine
{
    /* three reels, 64 stop positions each */
    short reel[NUM_REELS][REEL_STOPS];

    /* symbol showing on each reel after spin */
    short pos[NUM_REELS];
};

struct payline
{
    /* winning combo */
    char *desc;

    /* see if current reels == this winning combo */
    int (*pcheck_spin)();

    /* $ won, # times combo's won, running total $ won this combo */
    int award, hit_ctr, pay_ctr;
};

struct payline paylines[] =
{
    /* 15 different possible ways to win after each spin */
    { "red, white and blue 7s",       &red_white_blue_sevens,  2400 },
    { "three red 7s",                 &all_three_red_sevens,   1199 },
    { "three white 7s",               &all_three_white_sevens, 200  },
    { "three blue 7s",                &all_three_blue_sevens,  150  },
    { "any three 7s",                 &any_three_sevens,       80   },
    { "red bar, white bar, blue bar", &red_white_blue_bars,    50   },
    { "three blue bars",              &all_three_blue_bars,    40   },
    { "three white bars",             &all_three_white_bars,   25   },
    { "any red, white, blue",         &any_redwhiteblue,       20   },
    { "three red bars",               &all_three_red_bars,     10   },
    { "any three bars",               &any_three_bars,         5    },
    { "any three blue",               &any_three_blue,         2    },
    { "any three white",              &any_three_white,        2    },
    { "any three red",                &any_three_red,          2    },
    { "all spaces",                   &any_three_spaces,       1    }
};

struct slot_machine sm =
{{
    /* reel strip values, different per game, machine, etc. */
    {
        /* reel 1, red seven = 7 */
        1,1,1,1,2,2,2,2,
        1,1,3,3,3,3,1,5,
        5,5,5,1,1,1,1,7,
        1,1,1,1,5,5,5,5,
        1,3,3,3,3,1,2,2,
        2,2,1,3,3,3,3,1,
        5,5,5,5,1,1,6,6,
        1,1,1,1,4,4,4,4
    },
    {
        /* reel 2, white seven = 6 */
        1,1,1,1,2,2,2,1,
        1,1,4,4,4,4,1,5,
        5,5,5,1,1,1,1,6,
        1,1,1,1,5,5,5,5,
        1,4,4,4,4,1,1,2,
        2,1,1,4,4,4,4,1,
        5,5,5,5,1,1,7,7,
        1,1,1,1,3,3,3,3
    },
    {
        /* reel 3, blue seven = 2 */
        1,1,1,1,6,6,6,6,
        1,1,4,4,4,4,1,1,
        3,3,1,1,1,1,2,1,
        1,1,1,3,3,3,3,1,
        1,4,4,4,4,1,1,6,
        6,6,6,1,1,4,4,4,
        1,1,3,3,3,1,1,7,
        1,1,1,1,5,5,5,5
    }
}};

struct slot_machine *psm = &sm;
struct slot_machine *p = &sm;
struct payline *pline = paylines;

const short num_paylines = sizeof(paylines)/sizeof(struct payline);
const long handle_pulls = 10000000;
const int tick = 500000;

unsigned seed()
{
    time_t now = time(0);
    unsigned char *p = (unsigned char *)&now;
    unsigned seed = 0;
    size_t i;
    for (i=0; i<sizeof now; i++)
        seed = seed * (UCHAR_MAX + 2U) + p[i];

    return seed;
}

void usage()
{
    fprintf( stderr, "\n"
        "Slot Machine Analyzer\n"
        "Modeled after a real IGT Red, White & Blue\n"
        "georgeacraig@gmail.com\n\n");
}

void print_reel(int r)
{
    int i;
    printf("\nreel %d: ", r+1);
    for (i=0; i<REEL_STOPS; i++)
        printf("%d ", p->reel[r][i] );
}

void print_reels()
{
    int i;
    for (i=0; i<NUM_REELS; i++)
        print_reel(i);
}

void record_win(int n)
{
    /* hit counter, for calculating percentages */
    paylines[n].hit_ctr++;

    /* amount paid for each win */
    paylines[n].pay_ctr += paylines[n].award;
}

void calc_results()
{
    int i;
    float h = (float) handle_pulls;

    for (i=0; i<num_paylines; i++)
    {
        total_hits += paylines[i].hit_ctr;
        total_pays += paylines[i].pay_ctr;
    }

    payback_per = (total_pays / h) * 100.0F;
    hold_per = 100.0F - payback_per;
    hit_per = (total_hits / h) * 100.0F;
}

char *fmt_currency(unsigned long n)
{
    int pos, ctr;
    char* buf = malloc (255 * sizeof (char));

    sprintf(buf, "$ ");
    sprintf(&buf[2], "%d", n);
    pos = strlen(buf);

    for (;pos > 5; pos -= 3)
    {
        for (ctr = strlen(buf) + 1; ctr > pos - 4; buf[ctr+1]=buf[ctr--]);
        buf[++ctr] = ',';
    }

    return buf;
}

void display_results()
{
    printf("\n\nresults: (# wins/payout)\n\n");

    int i;
    for (i=0; i<num_paylines; i++)
    {
        char* payout = fmt_currency(paylines[i].pay_ctr);
        printf("%4d - %-28s = %-6d / %s\n",
            paylines[i].award,
            paylines[i].desc,
            paylines[i].hit_ctr,
            payout);
        free(payout);
    }

    char* ptotal_pays = fmt_currency(total_pays);
    char* phandle_pulls = fmt_currency(handle_pulls);

    printf(
        "\n"
        "totals:\n\n"
        "%17s = %2.1f%%\n"
        "%17s = %2.1f%%\n"
        "%17s = %s\n"
        "%17s = %s\n"
        "%17s = %2.1f%%\n"
        "%17s = %lu/%lu\n",
        "payback percent",   payback_per,
        "hold percent",      hold_per,
        "played (coin-in)",  phandle_pulls,
        "payout (coin-out)", ptotal_pays,
        "hit percent",       hit_per,
        "hits/total games",  total_hits, handle_pulls);

    free(ptotal_pays);
    free(phandle_pulls);
}

int compare_symbols(int x)
{
    return ((p->pos[0] == x) &
            (p->pos[1] == x) &
            (p->pos[2] == x));
}

int compare_colors(int x, int y)
{
    return (((p->pos[0] == x) | (p->pos[0] == y)) &
            ((p->pos[1] == x) | (p->pos[1] == y)) &
            ((p->pos[2] == x) | (p->pos[2] == y)));
}

int any_three_spaces()
{
    return compare_symbols(SYM_SPACE);
}

int any_three_blue()
{
    return compare_colors(SYM_BLUE_BAR, SYM_BLUE_SEVEN);
}

int any_three_white()
{
    return compare_colors(SYM_WHITE_BAR, SYM_WHITE_SEVEN);
}

int any_three_red()
{
    return compare_colors(SYM_RED_BAR, SYM_RED_SEVEN);
}

int any_three_bars()
{
    return ((
        (p->pos[0] == SYM_WHITE_BAR) |
        (p->pos[0] == SYM_BLUE_BAR) |
        (p->pos[0] == SYM_RED_BAR)) & (

        (p->pos[1] == SYM_WHITE_BAR) |
        (p->pos[1] == SYM_BLUE_BAR) |
        (p->pos[1] == SYM_RED_BAR)) & (

        (p->pos[2] == SYM_WHITE_BAR) |
        (p->pos[2] == SYM_BLUE_BAR) |
        (p->pos[2] == SYM_RED_BAR)));
}

int all_three_red_bars()
{
    return compare_symbols(SYM_RED_BAR);
}

int all_three_white_bars()
{
    return compare_symbols(SYM_WHITE_BAR);
}

int all_three_blue_bars()
{
    return compare_symbols(SYM_BLUE_BAR);
}

int all_three_blue_sevens()
{
    return compare_symbols(SYM_BLUE_SEVEN);
}

int all_three_white_sevens()
{
    return compare_symbols(SYM_WHITE_SEVEN);
}

int all_three_red_sevens()
{
    return compare_symbols(SYM_RED_SEVEN);
}

int red_white_blue_bars()
{
    return ((p->pos[0] == SYM_RED_BAR) &
            (p->pos[1] == SYM_WHITE_BAR) &
            (p->pos[2] == SYM_BLUE_BAR));
}

int any_redwhiteblue()
{
    return ((
            (p->pos[0] == SYM_RED_BAR) |
            (p->pos[0] == SYM_RED_SEVEN)) & (

            (p->pos[1] == SYM_WHITE_BAR) |
            (p->pos[1] == SYM_WHITE_SEVEN)) & (

            (p->pos[2] == SYM_BLUE_BAR) |
            (p->pos[2] == SYM_BLUE_SEVEN)));
}

int any_three_sevens()
{
    return ((
            (p->pos[0] == SYM_WHITE_SEVEN) |
            (p->pos[0] == SYM_BLUE_SEVEN) |
            (p->pos[0] == SYM_RED_SEVEN)) & (

            (p->pos[1] == SYM_WHITE_SEVEN) |
            (p->pos[1] == SYM_BLUE_SEVEN) |
            (p->pos[1] == SYM_RED_SEVEN)) & (

            (p->pos[2] == SYM_WHITE_SEVEN) |
            (p->pos[2] == SYM_BLUE_SEVEN) |
            (p->pos[2] == SYM_RED_SEVEN)));
}

int red_white_blue_sevens()
{
    return ((p->pos[0] == SYM_RED_SEVEN) &
            (p->pos[1] == SYM_WHITE_SEVEN) &
            (p->pos[2] == SYM_BLUE_SEVEN));
}

void check_spin()
{
    /* iterate through paylines checking current
       symbols and see if we've won */
    int i;
    for (i=0; i<num_paylines; i++)
        if ( paylines[i].pcheck_spin() )
        {
            /* if its a hit, tally and drop out for next spin */
            record_win(i);
            break;
        }
}

void run()
{
    int i,j;
    srand(seed());

    /*
     * -64 stop positions per reel
     * -3 reels
     * -64 * 64 * 64 = 262,144 total combinations
     * -odds of winning top payout = 1:262,144
     * -based on 10M handle pulls
     * -ridonkulous... play dice
     */

    for (i=0; i<handle_pulls; i++)
    {
        /* spinning! */

        for (j=0; j<NUM_REELS; j++)
        {
            /* randomly select a stop */
            int r = ((rand() % REEL_STOPS) + 1);

            /* move reel to symbol */
            p->pos[j] = p->reel[j][r];
        }

        /* print status */
        if ((i % tick) == 0)
        {
            printf(".");
            fflush(stdout);
        }

        check_spin();
    }

    calc_results();

    display_results();
}

int main(int ac, char **av)
{
    usage();
    run();
    return(0);
}
