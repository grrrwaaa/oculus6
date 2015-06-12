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

t_class *oculus_class;
static t_symbol * ps_quat;
static t_symbol * ps_pos;
static t_symbol * ps_warning;


class t_oculus {
public:
	t_object	ob;		
		
	t_oculus(int index = 0) {

	}
    
    ~t_oculus() {
		
	}
	
	
};

void oculus_free(t_oculus *x) {
    x->~t_oculus();
    
    // free resources associated with our obex entry
    //jit_ob3d_free(x);
    max_jit_object_free(x);
}

void *oculus_new(t_symbol *s, long argc, t_atom *argv)
{
    t_oculus *x = NULL;
	
    //int index = 0;
    //if (argc > 0 && atom_gettype(argv) == A_LONG) index = atom_getlong(argv);
    
    if ((x = (t_oculus *)object_alloc(oculus_class))) {
        
        // default attrs:
        // initialize in-place:
        x = new (x) t_oculus();
        
        // apply attrs:
        attr_args_process(x, argc, argv);
        
        // now configure:
        //oculus_configure(x);
    }
    return (x);
}

	
int C74_EXPORT main(void) {	
    t_class *maxclass;
	
	
	common_symbols_init();
    ps_quat = gensym("quat");
    ps_pos = gensym("pos");
	ps_warning = gensym("warning");
	
    maxclass = class_new("oculus", (method)oculus_new, (method)oculus_free, (long)sizeof(t_oculus),
                         0L, A_GIMME, 0);
    
    /*
	class_addmethod(maxclass, (method)oculus_assist, "assist", A_CANT, 0);
    class_addmethod(maxclass, (method)oculus_notify, "notify", A_CANT, 0);
    
    //class_addmethod(maxclass, (method)oculus_jit_matrix, "jit_matrix", A_GIMME, 0); 
    class_addmethod(maxclass, (method)oculus_bang, "bang", 0);
    class_addmethod(maxclass, (method)oculus_recenter, "recenter", 0);
    class_addmethod(maxclass, (method)oculus_dismiss, "dismiss", 0);
    class_addmethod(maxclass, (method)oculus_configure, "configure", 0);
    
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
	