#include "bbs.h"

// TODO move stuff to libbbs(or util)/string.c, ...
// this file can be removed (or not?)

int
expand_esc_star(char *buf, const char *src, int szbuf)
{
    assert(*src == KEY_ESC && *(src+1) == '*');
    src += 2;
    switch(*src)
    {
        //
        // secure content
        //
        case 't':   // current time
            strlcpy(buf, Cdate(&now), szbuf);
            return 1;
        case 'u':   // current online users
            snprintf(buf, szbuf, "%d", SHM->UTMPnumber);
            return 1;
        //
        // insecure content
        //
        case 's':   // current user id
            strlcpy(buf, cuser.userid, szbuf);
            return 2;
        case 'n':   // current user nickname
            strlcpy(buf, cuser.nickname, szbuf);
            return 2;
    }

    // unknown characters, return from star.
    strlcpy(buf, src-1, szbuf);
    return 0;
}

void
strip_ansi_movecmd(char *s) {
    const char *pattern_movecmd = "ABCDfjHJRu";
    const char *pattern_ansi_code = "0123456789;,[";

    while (*s) {
        char *esc = strchr(s, ESC_CHR);
        if (!esc)
            return;
        s = ++esc;
        while (*esc && strchr(pattern_ansi_code, *esc))
            esc++;
        if (strchr(pattern_movecmd, *esc)) {
            *esc = 's';
        }
    }
}

void
strip_esc_star(char *s) {
    int len;
    while (*s) {
        char *esc = strstr(s, ESC_STR "*");
        if (!esc)
            return;
        len = strlen(esc);
        // 3: ESC * [a-z]
        if (len < 3) {
            *esc = 0;
            return;
        }
        if (isalpha(esc[2])) {
            memmove(esc, esc + 3, len - 3 + 1);
        } else {
            // Invalid esc_star. Remove esc instead.
            memmove(esc, esc + 1, len - 1 + 1);
        }
    }
}

char *
Ptt_prints(char *str, size_t size, int mode)
{
    char           *strbuf = alloca(size);
    int             r, w;
    for( r = w = 0 ; str[r] != 0 && w < ((int)size - 1) ; ++r )
    {
        if( str[r] != ESC_CHR )
        {
            strbuf[w++] = str[r];
            continue;
        }

        if( str[++r] != '*' ){
            if (w+2 >= (int)size-1) break;
            strbuf[w++] = ESC_CHR;
            strbuf[w++] = str[r];
            continue;
        }

        /* Note, w will increased by copied length after */
        expand_esc_star(strbuf+w, &(str[r-1]), size-w);
        r ++;
        w += strlen(strbuf+w);
    }
    strbuf[w] = 0;
    strip_ansi(str, strbuf, mode);
    return str;
}

/* Jaky */
void
out_lines(const char *str, int line, int col)
{
    int y = vgety();
    move(y, col);
    while (*str && line) {
        if (*str == '\n')
        {
            move(++y, col);
            line--;
        } else
        {
            outc(*str);
        }
        str++;
    }
}

void
outmsg(const char *msg)
{
    move(b_lines - msg_occupied, 0);
    clrtoeol();
    outs(msg);
}

// vim:ts=4:expandtab
