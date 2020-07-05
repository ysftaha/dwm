#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 3;        /* border pixel of windows */
static const unsigned int minwsz    = 20;       /* Minimal heigt of a client for smfact */
static const unsigned int snap      = 0;        /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int focusonwheel       = 0;
static const char statussep         = ';';      /* separator between status bars */
static const char *fonts[]          = {"DejaVu Sans Mono:pixelsize=12:antialias=true:autohint=true"};
static const char dmenufont[]       = "DejaVu Sans Mono:pixelsize=12:antialias=true:autohint=true";
static const char col_gray1[]       = "#000000";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005743";
static const char *colors[][3]      = {
	//                         fg         bg         border
	[SchemeNorm]         = { col_gray3, col_gray1, col_gray1 },
	[SchemeSel]          = { col_gray4, col_cyan,  col_cyan  },
  [SchemeTabActive]    = { col_gray2, col_gray1,  col_gray2 },
  [SchemeTabInactive]  = { col_gray4, col_gray1,  col_gray1 }
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",         NULL,       NULL,       0,            1,           -1 },
	{ "qutebrowser",  NULL,       NULL,       1 << 3,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const float smfact    = 0.00; /* factor of tiled clients [0.00..0.95] */
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
	{ "[monocle]", monocle },
	{ "[tile]",       tile },
	{ "[float]",      NULL },
  { "[column]",      col },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },

	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.0125} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.0125} },

	{ MODKEY|ControlMask,           XK_k,      setsmfact,      {.f = +0.05} },
	{ MODKEY|ControlMask,           XK_j,      setsmfact,      {.f = -0.05} },

  { MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
  { MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },

	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },

	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_q,      killclient,     {0} },

	{ MODKEY,                       XK_space,  setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[3]} },
  { MODKEY,                       XK_c,      setlayout,      {.v = &layouts[5]} },
  { MODKEY,                       XK_n,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_f,      fullscreen,     {0} },
	{ MODKEY,                       XK_grave,  togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },

	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },

	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)

  { 0, XF86XK_MonBrightnessUp,	  spawn,		SHCMD("xbacklight -inc 1") },
	{ 0, XF86XK_MonBrightnessDown,	spawn,		SHCMD("xbacklight -dec 1") },

	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },

  /* FIXME monitor focus shit
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
  */
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	//{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {.v = &layouts[4]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

