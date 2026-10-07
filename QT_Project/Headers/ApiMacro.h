#ifdef QTCOMMONLIB_USE_DLL
#ifdef QTCOMMONLIB_BUILDING_PROJECT
#define QTCOMMONLIB_API __declspec(dllexport)
#else
#define QTCOMMONLIB_API __declspec(dllimport)
#endif
#else
#define QTCOMMONLIB_API
#endif
