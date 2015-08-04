#ifdef _WIN32
// Work-around main/SDL_main declaration conflict
// http://sourceforge.net/p/scummvm/bugs/3412/#c975 "TOOLS: MinGW tools building broken"
#ifdef main
#undef main
#endif
#endif
