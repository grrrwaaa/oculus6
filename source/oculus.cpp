/**
	@file
	oculus - a max object
 */

#ifdef __cplusplus
extern "C" {
#endif
#include "ext.h"
#include "ext_obex.h"
    
#include "z_dsp.h"
    
#include "jit.common.h"
#include "jit.gl.h"
#ifdef __cplusplus
}
#endif

#include <new>

#include "OVR_CAPI_0_6_0.h"
#include "OVR_CAPI_GL.h"

t_class *oculus_class;
static t_symbol * ps_quat;
static t_symbol * ps_pos;
static t_symbol * ps_warning;


class t_oculus {
public:
	t_object	ob;	
	void * ob3d;	
		
	void * outlet_msg;

	// LibOVR objects:
    ovrHmd		hmd;
		
	t_oculus(t_symbol * dest_name, int index = 0) {
		jit_ob3d_new(this, dest_name);
		outlet_msg = outlet_new(this, 0);

		hmd = 0;

		/*
		ovrResult result = ovrHmd_Create(index, &hmd);
		if (result != ovrSuccess) {
			object_warn(&ob, "failed to create HMD instance");
			hmd = 0;
		}
		*/
		/*
		unsigned int hmdCaps = 0;
        
		if (hmd_count <= 0) {
			object_warn(&ob, "no HMD detected");
		} else if (index >= hmd_count) {
			object_warn(&ob, "request for HMD %d out of range (only %d devices available)", index, hmd_count);
		} else {
			ovrResult result = ovrHmd_Create(index, &hmd);
			if (result != ovrSuccess) {
				object_warn(&ob, "failed to create HMD instance");
				hmd = 0;
			} else {
				hmdCaps = ovrHmd_GetEnabledCaps(hmd);
				//if (!(hmdCaps & ovrHmdCap_Available)) {
				//	object_error(&ob, "HMD in use by another application");
				//	ovrHmd_Destroy(hmd);
				//	hmd = 0;
				//} else 
				//if (!(hmdCaps & ovrHmdCap_ExtendDesktop)) {
				//	object_error(&ob, "Please switch the Oculus Rift driver to extended desktop mode");
				//	ovrHmd_Destroy(hmd);
				//	hmd = 0;
				//}
			}
		}
        if (hmd == 0) {
            object_warn(&ob, "HMD not acquired, using offline DK2 simulator instead");
            ovrResult result = ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
			if (result != ovrSuccess) {
				hmd = 0;
				object_error(&ob, "fatal error creating HMD");
				return;
			}
		}
		*/

		ovrResult result = ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
		if (result != ovrSuccess) {
			hmd = 0;
			object_error(&ob, "fatal error creating HMD");
			return;
		}
		
	}
    
    ~t_oculus() {
		jit_ob3d_free(this);
		
		if (hmd) {
			ovrHmd_Destroy(hmd);
		}

		max_jit_object_free(this);
	}
	
	void configure() {

	}

	void bang() {

	}

	t_jit_err dest_changed() {
		
		return JIT_ERR_NONE;
	}
	
	t_jit_err draw() {
		return JIT_ERR_NONE;
	}
	
	t_jit_err dest_closing() {	
		
		
		return JIT_ERR_NONE;
	}
	
	t_jit_err ui(t_line_3d *p_line, t_wind_mouse_info *p_mouse) {
		/*
		post("line (%f,%f,%f)-(%f,%f,%f); mouse(%s)",
			p_line->u[0], p_line->u[1], p_line->u[2], 
			p_line->v[0], p_line->v[1], p_line->v[2], 
			p_mouse->mousesymbol->s_name			// mouse, mouseidle
		);
		*/
		return JIT_ERR_NONE;
	}
	
};

t_jit_err oculus_draw(t_oculus * x) { return x->draw(); }
t_jit_err oculus_ui(t_oculus * x, t_line_3d *p_line, t_wind_mouse_info *p_mouse) { return x->ui(p_line, p_mouse); }
t_jit_err oculus_dest_closing(t_oculus * x) { return x->dest_closing(); }
t_jit_err oculus_dest_changed(t_oculus * x) { return x->dest_changed(); }

void oculus_bang(t_oculus * x) {
    x->bang();
}

void oculus_doconfigure(t_oculus *x) {
    x->configure();
}

void oculus_configure(t_oculus *x) {
    defer_low(x, (method)oculus_doconfigure, 0, 0, 0);
}


void oculus_free(t_oculus *x) {
    x->~t_oculus();
    
    // free resources associated with our obex entry
    //jit_ob3d_free(x);
    //max_jit_object_free(x);
}

void *oculus_new(t_symbol *s, long argc, t_atom *argv)
{
    t_oculus *x = NULL;
	
    //int index = 0;
    //if (argc > 0 && atom_gettype(argv) == A_LONG) index = atom_getlong(argv);
    
    if ((x = (t_oculus *)object_alloc(oculus_class))) {
		
		// get context:
		t_symbol * dest_name = atom_getsym(argv);
        
        // default attrs:
        // initialize in-place:
        x = new (x) t_oculus(dest_name);
        
        // apply attrs:
        attr_args_process(x, argc, argv);
        
        // now configure:
        oculus_configure(x);
    }
    return (x);
}
	

void oculus_quit() {
	ovr_Shutdown();
}

void oculus_log(int level, const char* message) {
	post("oculus log %d %s", level, message);
}

int C74_EXPORT main(void) {	
    t_class *maxclass;
	
	common_symbols_init();
    ps_quat = gensym("quat");
    ps_pos = gensym("pos");
	ps_warning = gensym("warning");

	ovrInitParams params;
	params.Flags = 0;
	params.RequestedMinorVersion = 0;
	params.LogCallback = oculus_log;
	params.ConnectionTimeoutMS = 0;

	// LibOVRRT32_0_6.dll
	ovrResult result = ovr_Initialize(0);
	if (result != ovrSuccess) {
		object_error(NULL, "LibOVR: failed to initialize library");
		
		switch (result) {
			case ovrError_Initialize: object_error(NULL, "Generic initialization error."); break;
			case ovrError_LibLoad: object_error(NULL, "Couldn't load LibOVRRT."); break;
			case ovrError_LibVersion: object_error(NULL, "LibOVRRT version incompatibility."); break;
			case ovrError_ServiceConnection: object_error(NULL, "Couldn't connect to the OVR Service."); break;
			case ovrError_ServiceVersion: object_error(NULL, "OVR Service version incompatibility."); break;
			case ovrError_IncompatibleOS: object_error(NULL, "The operating system version is incompatible."); break;
			case ovrError_DisplayInit: object_error(NULL, "Unable to initialize the HMD display."); break;
			case ovrError_ServerStart:  object_error(NULL, "Unable to start the server. Is it already running?"); break;
			case ovrError_Reinitialization: object_error(NULL, "Attempted to re-initialize with a different version."); break;
			default: object_error(NULL, "unknown initialization error."); break;
		}

		/*
		// was crashy:
		ovrErrorInfo errInfo;
		ovr_GetLastErrorInfo(&errInfo);
		object_error(NULL, errInfo.ErrorString);
		*/
		return 0;
	}
	object_post(NULL, "initialized LibOVR %s", ovr_GetVersionString());
	//quittask_install((method)oculus_quit, NULL);

	//int32_t hmd_count = ovrHmd_Detect();
	//object_post(NULL, "%d HMDs detected", hmd_count);
	
    maxclass = class_new("oculus", (method)oculus_new, (method)oculus_free, (long)sizeof(t_oculus),
                         0L, A_GIMME, 0);
    
	long ob3d_flags = JIT_OB3D_NO_MATRIXOUTPUT 
					| JIT_OB3D_DOES_UI;
	void * ob3d = jit_ob3d_setup(maxclass, calcoffset(t_oculus, ob3d), ob3d_flags);
	// define our OB3D draw methods
	jit_class_addmethod(maxclass, (method)(oculus_draw), "ob3d_draw", A_CANT, 0L);
	jit_class_addmethod(maxclass, (method)(oculus_dest_closing), "dest_closing", A_CANT, 0L);
	jit_class_addmethod(maxclass, (method)(oculus_dest_changed), "dest_changed", A_CANT, 0L);
	if (ob3d_flags & JIT_OB3D_DOES_UI) {
		jit_class_addmethod(maxclass, (method)(oculus_ui), "ob3d_ui", A_CANT, 0L);
	}
	// must register for ob3d use
	jit_class_addmethod(maxclass, (method)jit_object_register, "register", A_CANT, 0L);
	
	
	class_addmethod(maxclass, (method)oculus_bang, "bang", 0);
    class_addmethod(maxclass, (method)oculus_configure, "configure", 0);
    /*
	class_addmethod(maxclass, (method)oculus_assist, "assist", A_CANT, 0);
    class_addmethod(maxclass, (method)oculus_notify, "notify", A_CANT, 0);
    
    //class_addmethod(maxclass, (method)oculus_jit_matrix, "jit_matrix", A_GIMME, 0); 
    
    class_addmethod(maxclass, (method)oculus_recenter, "recenter", 0);
    class_addmethod(maxclass, (method)oculus_dismiss, "dismiss", 0);
    
    CLASS_ATTR_FLOAT(maxclass, "predict", 0, t_oculus, predict);
	
    CLASS_ATTR_LONG(maxclass, "fullview", 0, t_oculus, fullview);
	CLASS_ATTR_ACCESSORS(maxclass, "fullview", NULL, oculus_fullview_set);
    CLASS_ATTR_STYLE_LABEL(maxclass, "fullview", 0, "onoff", "use default (0) or max (1) field of view");
	
	CLASS_ATTR_LONG(maxclass, "lowpersistence", 0, t_oculus, lowpersistence);
	CLASS_ATTR_ACCESSORS(maxclass, "lowpersistence", NULL, oculus_lowpersistence_set);
	CLASS_ATTR_STYLE_LABEL(maxclass, "lowpersistence", 0, "onoff", "enable low persistence mode (may reduce judder)");
	
	CLASS_ATTR_LONG(maxclass, "dynamicprediction", 0, t_oculus, dynamicprediction);
	CLASS_ATTR_ACCESSORS(maxclass, "dynamicprediction", NULL, oculus_dynamicprediction_set);
	CLASS_ATTR_STYLE_LABEL(maxclass, "dynamicprediction", 0, "onoff", "enable dynamic prediction (may improve tracking)");

	CLASS_ATTR_LONG(maxclass, "warning", 0, t_oculus, warning);
    CLASS_ATTR_STYLE_LABEL(maxclass, "warning", 0, "onoff", "show health & safety warning");
    */
	
    class_register(CLASS_BOX, maxclass); 
    oculus_class = maxclass;
    return 0;
}
	