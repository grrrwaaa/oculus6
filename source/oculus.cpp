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

	void * outlet_q;
	void * outlet_p;
	void * outlet_p_info;
	void * outlet_msg;

	// the quaternion orientation of the HMD:
	t_atom		quat[4];
	t_atom		pos[3];

	// attrs:
    int			fullview;
    double      predict;    // time (in ms) to predict ahead tracking
	int			lowpersistence;
	int			dynamicprediction;

	// LibOVR objects:
    ovrHmd		hmd;
		
	t_oculus(t_symbol * dest_name, int index = 0) {
		jit_ob3d_new(this, dest_name);

		outlet_msg = outlet_new(this, 0);
		outlet_p_info = outlet_new(&ob, 0);
        outlet_p = listout(&ob);
        outlet_q = listout(&ob);

		atom_setfloat(quat+0, 0);
        atom_setfloat(quat+1, 0);
        atom_setfloat(quat+2, 0);
        atom_setfloat(quat+3, 1);
        
        atom_setfloat(pos+0, 0.f);
        atom_setfloat(pos+1, 0.f);
        atom_setfloat(pos+2, 0.f);

		hmd = 0;
		fullview = 1;
		lowpersistence = 1;
		dynamicprediction = 0;
		
		ovrResult result;
		int32_t hmd_count = ovrHmd_Detect();
		object_post(NULL, "%d HMDs detected", hmd_count);
		
		unsigned int hmdCaps = 0;
        
		if (hmd_count <= 0) {
			object_warn(&ob, "no HMD detected");
		} else if (index >= hmd_count) {
			object_warn(&ob, "request for HMD %d out of range (only %d devices available)", index, hmd_count);
		} else {
			result = ovrHmd_Create(index, &hmd);
			if (result != ovrSuccess) {
				object_warn(&ob, "failed to create HMD instance");
				hmd = 0;
			} else {
				object_post(&ob, "acquired HMD");
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
            result = ovrHmd_CreateDebug(ovrHmd_DK2, &hmd);
			if (result != ovrSuccess) {
				hmd = 0;
				object_error(&ob, "fatal error creating HMD");
				return;
			}
		}

		result = ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation|ovrTrackingCap_MagYawCorrection|ovrTrackingCap_Position, 0);
		if (result != ovrSuccess) {
			object_error(&ob, "problem enabling tracking");
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
		if (!hmd) {
			object_warn(&ob, "No HMD to configure"); 
			return;
		}

		t_atom a[4];

		unsigned int hmdCaps = 0;
		if (lowpersistence) hmdCaps |= ovrHmdCap_LowPersistence;
		if (dynamicprediction) hmdCaps |= ovrHmdCap_DynamicPrediction;
		ovrHmd_SetEnabledCaps(hmd, hmdCaps);
		hmdCaps = ovrHmd_GetEnabledCaps(hmd);
		lowpersistence = (hmdCaps & ovrHmdCap_LowPersistence) ? 1: 0;
		dynamicprediction = (hmdCaps & ovrHmdCap_DynamicPrediction) ? 1 : 0;

		// note serial:
        atom_setsym(a, gensym(hmd->SerialNumber));
        outlet_anything(outlet_msg, gensym("serial"), 1, a);

		   
    #define HMD_CASE(T) case T: { \
            atom_setsym(a, gensym( #T )); \
            outlet_anything(outlet_msg, gensym("hmdType"), 1, a); \
            break; \
        }
        switch(hmd->Type) {
                HMD_CASE(ovrHmd_DK1)
                HMD_CASE(ovrHmd_DKHD)
                HMD_CASE(ovrHmd_DK2)
            default: {
                atom_setsym(a, gensym("unknown"));
                outlet_anything(outlet_msg, gensym("Type"), 1, a);
            }
        }
    #undef HMD_CASE
        
        atom_setsym(a, gensym(hmd->Manufacturer));
        outlet_anything(outlet_msg, gensym("Manufacturer"), 1, a);
        atom_setsym(a, gensym(hmd->ProductName));
        outlet_anything(outlet_msg, gensym("ProductName"), 1, a);
        
        atom_setlong(a, hmd->FirmwareMajor);
        atom_setlong(a+1, hmd->FirmwareMinor);
        outlet_anything(outlet_msg, gensym("Firmware"), 2, a);
        
        atom_setfloat(a, hmd->CameraFrustumHFovInRadians);
        outlet_anything(outlet_msg, gensym("CameraFrustumHFovInRadians"), 1, a);
        atom_setfloat(a, hmd->CameraFrustumVFovInRadians);
        outlet_anything(outlet_msg, gensym("CameraFrustumVFovInRadians"), 1, a);
        atom_setfloat(a, hmd->CameraFrustumNearZInMeters);
        outlet_anything(outlet_msg, gensym("CameraFrustumNearZInMeters"), 1, a);
        atom_setfloat(a, hmd->CameraFrustumFarZInMeters);
        outlet_anything(outlet_msg, gensym("CameraFrustumFarZInMeters"), 1, a);
        
        atom_setlong(a, hmd->Resolution.w);
        atom_setlong(a+1, hmd->Resolution.h);
        outlet_anything(outlet_msg, gensym("Resolution"), 2, a);
        
        atom_setlong(a, hmd->EyeRenderOrder[0]);
        outlet_anything(outlet_msg, gensym("EyeRenderOrder"), 1, a);

/*
    unsigned int HmdCaps;                      ///< Capability bits described by ovrHmdCaps.
    unsigned int TrackingCaps;                 ///< Capability bits described by ovrTrackingCaps.
    ovrFovPort   DefaultEyeFov[ovrEye_Count];  ///< Defines the recommended FOVs for the HMD.
    ovrFovPort   MaxEyeFov[ovrEye_Count];      ///< Defines the maximum FOVs for the HMD.
  */
        atom_setlong(a, hmdCaps & lowpersistence);
        outlet_anything(outlet_msg, gensym("lowpersistence"), 1, a);
        atom_setlong(a, hmdCaps & dynamicprediction);
        outlet_anything(outlet_msg, gensym("dynamicprediction"), 1, a);
	}

	void pose(double predict_ms = 0.) {
		if (!hmd) return;
		ovrTrackingState ts = ovrHmd_GetTrackingState(hmd, ovr_GetTimeInSeconds() + (predict_ms * 0.001));
		
		const ovrPoseStatef& predicted = ts.HeadPose;

		//if (ts.StatusFlags & ovrStatus_PositionConnected) {
		// this can be zero if the HMD is outside the camera frustum
		// or is facing away from it
		// or partially occluded
		// or it is moving too fast
		if (ts.StatusFlags & ovrStatus_PositionTracked) {
			outlet_int(outlet_p_info, (t_atom_long)1);

			// axis system of camera has ZX always parallel to ground (presumably by gravity)
			// default origin is 1m in front of the camera (in +Z), but at the same height (even if camera is tilted)
			
			/*
			 typedef struct ovrPoseStatef_
			 {
			 ovrPosef     ThePose;
			 ovrVector3f  AngularVelocity;
			 ovrVector3f  LinearVelocity;
			 ovrVector3f  AngularAcceleration;
			 ovrVector3f  LinearAcceleration;
			 double       TimeInSeconds;         // Absolute time of this state sample.
			 } ovrPoseStatef;
			 */
			
			const ovrVector3f position = predicted.ThePose.Position;
			
			atom_setfloat(pos  , position.x);
			atom_setfloat(pos+1, position.y);
			atom_setfloat(pos+2, position.z);
			outlet_list(outlet_p, 0L, 3, pos);
			
			// TODO: accessors for these:
			// CameraPose is the pose of the camera relative to the origin
			// LeveledCameraPose is the same but with roll & pitch zeroed out
		} else {
			outlet_int(outlet_p_info, (t_atom_long)0);
			// is there some kind of predictive interpolation we can use here?
			
			outlet_list(outlet_p, 0L, 3, pos);
		}

		if (ts.StatusFlags & ovrStatus_OrientationTracked) {
			const ovrQuatf& orient = predicted.ThePose.Orientation;
			
            atom_setfloat(quat  , orient.x);
            atom_setfloat(quat+1, orient.y);
            atom_setfloat(quat+2, orient.z);
            atom_setfloat(quat+3, orient.w);

			outlet_list(outlet_q, 0L, 4, quat);

			
		}

	}

	void bang() {
		pose();
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

void oculus_recenter(t_oculus * x) {
    if (x->hmd) ovrHmd_RecenterPose(x->hmd);
}

void oculus_doconfigure(t_oculus *x) {
    x->configure();
}

void oculus_configure(t_oculus *x) {
    defer_low(x, (method)oculus_doconfigure, 0, 0, 0);
}

t_max_err oculus_fullview_set(t_oculus *x, t_object *attr, long argc, t_atom *argv) {
    x->fullview = atom_getlong(argv);
    
    oculus_configure(x);
    return 0;
}

t_max_err oculus_lowpersistence_set(t_oculus *x, t_object *attr, long argc, t_atom *argv) {
	x->lowpersistence = atom_getlong(argv);
	
	oculus_configure(x);
	return 0;
}

t_max_err oculus_dynamicprediction_set(t_oculus *x, t_object *attr, long argc, t_atom *argv) {
	x->dynamicprediction = atom_getlong(argv);
	
	oculus_configure(x);
	return 0;
}

void oculus_assist(t_oculus *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) { // inlet
        if (a == 0) {
            sprintf(s, "messages in, bang to report orientation");
        } else {
            sprintf(s, "I am inlet %ld", a);
        }
    } else {	// outlet
        if (a == 0) {
            sprintf(s, "HMD orientation quaternion (list)");
        } else if (a == 1) {
            sprintf(s, "HMD position (list)");
        } else if (a == 2) {
            sprintf(s, "HMD position tracking status (messages)");
        } else if (a == 3) {
            sprintf(s, "HMD left eye properties (messages)");
        } else if (a == 4) {
            sprintf(s, "HMD left eye mesh (jit_matrix)");
        } else if (a == 5) {
            sprintf(s, "HMD right eye properties (messages)");
        } else if (a == 6) {
            sprintf(s, "HMD right eye mesh (jit_matrix)");
        } else if (a == 7) {
            sprintf(s, "HMD properties (messages)");
        } else {
            sprintf(s, "I am outlet %ld", a);
        }
    }
}

void oculus_free(t_oculus *x) {
    x->~t_oculus();
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
	quittask_install((method)oculus_quit, NULL);
	
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
	class_addmethod(maxclass, (method)oculus_assist, "assist", A_CANT, 0);
    /*
    class_addmethod(maxclass, (method)oculus_notify, "notify", A_CANT, 0);
    
    //class_addmethod(maxclass, (method)oculus_jit_matrix, "jit_matrix", A_GIMME, 0); 
      */
    class_addmethod(maxclass, (method)oculus_recenter, "recenter", 0);

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

 
	
    class_register(CLASS_BOX, maxclass); 
    oculus_class = maxclass;
    return 0;
}
	