#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int gappx     = 5;        /* gaps between windows */
static const unsigned int snap      = 0;        /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = {
  "DejaVu Sans Mono:pixelsize=12:antialias=true:autohint=true",
  "Font Awesome 5 Free:pixelsize=13:antialias=true:autohint=true", 
};

static const char col_gray1[]       = "#000000";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005743";
static const char *colors[][3]      = {
  /*                         fg         bg         border */
  [SchemeNorm]         = { col_gray3, col_gray1,  col_gray1 },
  [SchemeSel]          = { col_gray4, col_cyan,   col_cyan  },
  [SchemeTabActive]    = { col_gray2, col_gray1,  col_gray2 },
  [SchemeTabInactive]  = { col_gray4, col_gray1,  col_gray1 }
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

/* use xprop to set class */
static const Rule rules[] = {
  /* class             instance    title       tags mask     isfloating   monitor */
  { "brave-browser",   NULL,       NULL,       1 << 2,       0,           -1 },
  { "Brave-browser",   NULL,       NULL,       1 << 2,       0,           -1 },

  {"org.pwmt.zathura", NULL,       NULL,       1 << 1,       0,           -1 },

  {"Anki",             NULL,       NULL,       1 << 3,       0,           -1 },
  {"anki",             NULL,       NULL,       1 << 3,       0,           -1 }
};

/* layout(s) */
static const float mfact     = 0.33; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

/* Bartabgroups properties */
#define BARTAB_BORDERS 1       // 0 = off, 1 = on
#define BARTAB_BOTTOMBORDER 1  // 0 = off, 1 = on
#define BARTAB_TAGSINDICATOR 1 // 0 = off, 1 = on if >1 client/view tag, 2 = always on
#define BARTAB_TAGSPX 5        // # pixels for tag grid boxes
#define BARTAB_TAGSROWS 3      // # rows in tag grid (9 tags, e.g. 3x3)
static void (*bartabmonfns[])(Monitor *) = { monocle /* , customlayoutfn */ };
static void (*bartabfloatfns[])(Monitor *) = { NULL /* , customlayoutfn */ };

static const Layout layouts[] = {
  /* symbol     arrange function */
  { "[deck]",       deck },
  { "[cMaster]", centeredmaster},
  { "[fullscr]", monocle },
  { "[tile]",       tile },
  { "[column]",      col },
  { "[float]",      NULL },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG,VIEW) \
{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
{ MODKEY,                       VIEW,     toggleview,     {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             VIEW,     toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run"};
static const char *termcmd[]  = { "st", NULL };

static const char scratchpadname[] = "scratchpad"; 
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "128x40", NULL }; 

#include "movestack.c"
static Key keys[] = {
  /* modifier                     key        function        argument */
  { MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
  { MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },

  /* windows */
  { MODKEY|ShiftMask,             XK_space,  zoom,           {0} },
  { MODKEY,                       XK_space,  switchcol,      {0} },

  /* the stack */
  { MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
  { MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
  { MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
  { MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },

  /* the master window */
  { MODKEY,                       XK_h,      setmfact,       {.f = -0.0050} },
  { MODKEY,                       XK_l,      setmfact,       {.f = +0.0050} },
  { MODKEY,                       XK_backslash,  switchfact,     {0}        },
  { MODKEY|ShiftMask,             XK_i,      incnmaster,     {.i = +1 } },
  { MODKEY|ShiftMask,             XK_d,      incnmaster,     {.i = -1 } },

  /* fact resize */
  { MODKEY|ControlMask,           XK_k,      setcfact,       {.f = +0.25} },
  { MODKEY|ControlMask,           XK_j,      setcfact,       {.f = -0.25} },
  { MODKEY|ControlMask,           XK_0,      setcfact,       {.f =  0.00} },

  /* Layouts */
  { MODKEY,                       XK_d,      setlayout,      {.v = &layouts[0]} }, // deck
  { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[1]} }, // Master
  { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[3]} }, // Tile
  { MODKEY,                       XK_c,      setlayout,      {.v = &layouts[4]} }, // Column
  { MODKEY,                       XK_f,      setlayout,      {.v = &layouts[2]} }, // Fullscr

  /* Tags */
  TAGKEYS(XK_1,    0, XK_F1)
    TAGKEYS(XK_2,    1, XK_F2)
    TAGKEYS(XK_3,    2, XK_F3)
    TAGKEYS(XK_4,    3, XK_F4)
    TAGKEYS(XK_5,    4, XK_F5)
    TAGKEYS(XK_6,    5, XK_F6)
    TAGKEYS(XK_7,    6, XK_F7)
    TAGKEYS(XK_8,    7, XK_F8)
    TAGKEYS(XK_9,    8, XK_F9)
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },

    /* quick utils */
    { MODKEY,                       XK_Tab,    view,           {0} },
    { MODKEY,                       XK_b,      togglebar,      {0} },
    { MODKEY,                       XK_q,      killclient,     {0} },
    { MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
    { MODKEY,                       XK_Print,	 spawn,		       SHCMD("maim $HOME/sc-shot.png") },
    { MODKEY,                       XK_s,	     spawn,		       SHCMD("surf lobste.rs") },

    { 0,                            XF86XK_AudioMute,       	 spawn,		 SHCMD("vol mute") },
    { 0,                            XF86XK_AudioRaiseVolume, 	 spawn,		 SHCMD("vol up") },
    { 0,                            XF86XK_AudioLowerVolume,	 spawn,		 SHCMD("vol down") },

    /* quit */
    { MODKEY|ShiftMask|ControlMask, XK_q,      quit,           {0} },

    { MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
    { MODKEY,                       XK_period, focusmon,       {.i = +1 } },
    { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
    { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
  /* click                event mask      button          function        argument */
  { ClkClientWin,         MODKEY|ShiftMask,         Button1,        movemouse,      {0} },
  { ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[5]} },
  { ClkWinTitle,          0,              Button2,        zoom,           {0} },
  { ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
  { ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
  { ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
  { ClkTagBar,            0,              Button1,        view,           {0} },
  { ClkTagBar,            0,              Button3,        toggleview,     {0} },
  { ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
  { ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

