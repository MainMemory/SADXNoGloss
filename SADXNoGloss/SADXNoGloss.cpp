// SADXNoGloss.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <vector>
#include "..\sadx-mod-loader\SADXModLoader\include\SADXModLoader.h"

using std::vector;

enum arraytype { arraytype_objects, arraytype_models, arraytype_actions };

struct arrayinfo { const char *name; int length; int type; };

arrayinfo arraylist[] = {
	{ "___SONIC_OBJECTS", 79, arraytype_objects },
	{ "___SONIC_ACTIONS", 149, arraytype_actions },
	{ "___SONIC_MODELS", 11, arraytype_models },
	{ "___MILES_OBJECTS", 72, arraytype_objects },
	{ "___MILES_ACTIONS", 114, arraytype_actions },
	{ "___MILES_MODELS", 15, arraytype_models },
	{ "___KNUCKLES_OBJECTS", 74, arraytype_objects },
	{ "___KNUCKLES_ACTIONS", 90, arraytype_actions },
	{ "___KNUCKLES_MODELS", 24, arraytype_models },
	{ "___AMY_OBJECTS", 39, arraytype_objects },
	{ "___AMY_ACTIONS", 80, arraytype_actions },
	{ "___AMY_MODELS", 6, arraytype_models },
	{ "___E102_OBJECTS", 28, arraytype_objects },
	{ "___E102_ACTIONS", 78, arraytype_actions },
	{ "___BIG_OBJECTS", 46, arraytype_objects },
	{ "___BIG_ACTIONS", 90, arraytype_actions },
	{ "___BIG_MODELS", 4, arraytype_models }
};

vector<void*> visitedaddrs;

bool checkaddrvisited(void *addr)
{
	for (size_t i = 0; i < visitedaddrs.size(); i++)
		if (addr == visitedaddrs[i])
			return true;
	visitedaddrs.push_back(addr);
	return false;
}

void processmodel(NJS_MODEL_SADX *mdl)
{
	if (checkaddrvisited(mdl))
		return;
	if (mdl->mats && mdl->nbMat)
		for (int i = 0; i < mdl->nbMat; i++)
			mdl->mats[i].specular.color = 0;
}

void processobject(NJS_OBJECT *obj)
{
	do {
		if (checkaddrvisited(obj))
			return;
		if (obj->basicdxmodel)
			processmodel(obj->basicdxmodel);
		if (obj->child)
			processobject(obj->child);
		obj = obj->sibling;
	} while (obj);
}

extern "C"
{
	__declspec(dllexport) void Init(const char *path, const HelperFunctions &helperFunctions)
	{
		HMODULE hmodule = GetModuleHandle(L"CHRMODELS_orig");
		for (size_t i = 0; i < LengthOfArray(arraylist); i++)
		{
			void *listaddr = (void *)GetProcAddress(hmodule, arraylist[i].name);
			switch (arraylist[i].type)
			{
			case arraytype_objects:
			{
				NJS_OBJECT **objlist = (NJS_OBJECT **)listaddr;
				for (int j = 0; j < arraylist[i].length; j++)
				{
					if (*objlist != nullptr)
						processobject(*objlist);
					++objlist;
				}
				break;
			}
			case arraytype_actions:
			{
				NJS_ACTION **actlist = (NJS_ACTION **)listaddr;
				for (int j = 0; j < arraylist[i].length; j++)
				{
					if (*actlist != nullptr)
						processobject((*actlist)->object);
					++actlist;
				}
				break;
			}
			case arraytype_models:
			{
				NJS_MODEL_SADX **mdllist = (NJS_MODEL_SADX **)listaddr;
				for (int j = 0; j < arraylist[i].length; j++)
				{
					if (*mdllist != nullptr)
						processmodel(*mdllist);
					++mdllist;
				}
				break;
			}
			}
		}
	}

	__declspec(dllexport) ModInfo SADXModInfo = { ModLoaderVer };
}