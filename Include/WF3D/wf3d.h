#ifndef WF3D_H_INCLUDED
#define WF3D_H_INCLUDED

#ifdef WF3D_BUILDING_WITH_MSVC
	#ifdef WF3D_BUILDING
		#define WF3D_DLL_EXPORT __declspec(dllexport)
	#else
		#define WF3D_DLL_EXPORT __declspec(dllimport)
	#endif
#else
	#define WF3D_DLL_EXPORT
#endif

#endif // WF3D_H_INCLUDED
