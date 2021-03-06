// Copyright (c) 2013, Zhang Yungui, https://github.com/rhcad/touchvg

#ifdef SWIGJAVA
%javaconst(1);
%insert("runtime") %{
#ifdef __ANDROID__
#define SWIG_JAVA_NO_DETACH_CURRENT_THREAD
#define SWIG_JAVA_ATTACH_CURRENT_THREAD_AS_DAEMON
#endif
%}
#endif

%module(directors="1") touchvg

%{
#include <mgbox.h>
#include <mgmat.h>
#include <mgbase.h>
#include <mgcurv.h>
#include <mglnrel.h>
#include <mgnear.h>

#include <gigraph.h>
#include <gicanvas.h>
#include <gipath.h>

#include <mgstorage.h>
#include <mgvector.h>

#include <mgshapes.h>
#include <mgbasicsp.h>
#include <mgcomposite.h>
#include <mggrid.h>
#include <mgpathsp.h>
#include <mgshapetype.h>
#include <mgstorage.h>
#include <mgspfactory.h>

#include <mgsnap.h>
#include <mgselect.h>
#include <mgaction.h>
#include <mgcmd.h>
#include <cmdsubject.h>

#include <mgdrawarc.h>
#include <mgdrawrect.h>

#include <mgcmdmgr.h>
#include <mgcmdmgrfactory.h>

#include <mglayer.h>
#include <mgshapedoc.h>

#include "gicoreview.h"
#include "gimousehelper.h"
#include "testcanvas.h"
#include "giplaying.h"
#include "gicoreviewdata.h"
%}

%include <mgdef.h>
%include <mgtol.h>
%include <mgvec.h>
%include <mgpnt.h>
%include <mgmat.h>
%include <mgbox.h>

%include <mgbase.h>
%include <mgcurv.h>
%include <mglnrel.h>
%include <mgnear.h>

%feature("director") GiCanvas;
%include <gicanvas.h>

%include <gicolor.h>
%include <gicontxt.h>
%include <gixform.h>
%include <gipath.h>
%include <gigraph.h>

%include <mgvector.h>
%template(Ints) mgvector<int>;
%template(Longs) mgvector<long>;
%template(Floats) mgvector<float>;
%template(Chars) mgvector<char>;
%template(ConstShapes) mgvector<const MgShape*>;
%template(Shapes) mgvector<MgShape*>;

%include <mgstorage.h>

%feature("director") MgObject;
%feature("director") MgBaseShape;
%feature("director") MgBaseRect;
%feature("director") MgBaseLines;
%feature("director") MgComposite;
%include <mgobject.h>
%include <mgshape.h>
%include <mgspfactory.h>
%include <mgshapes.h>
%include <mgbasicsp.h>
%include <mgcomposite.h>
%include <mggrid.h>
%include <mgpathsp.h>
%include <mgshapetype.h>
%include <mgspfactory.h>

%include <mgaction.h>
%include <mgsnap.h>
%include <mgselect.h>
%include <mgview.h>

%feature("director") MgCommand;
%include <mgcmd.h>

%feature("director") CmdObserverDefault;
%include <cmdobserver.h>
%include <cmdsubject.h>

%feature("director") MgCommandDraw;
%feature("director") MgCmdDrawRect;
%include <mgcmddraw.h>
%include <mgdrawarc.h>
%include <mgdrawrect.h>

%include <mgcmdmgr.h>
%include <mgcmdmgrfactory.h>

%include <mglayer.h>
%include <mgshapedoc.h>

%feature("director") GiView;
%include "giview.h"

%feature("director") MgFindImageCallback;
%feature("director") MgStringCallback;
%include "mgcoreview.h"
%include "gigesture.h"
%include "gicoreview.h"
%include "testcanvas.h"
%include "giplaying.h"
%include "gicoreviewdata.h"
%include "recordshapes.h"

#ifndef __ANDROID__
%include "gimousehelper.h"
#endif

#ifdef SWIGJAVA
%{
struct TmpJOBJ {
    JNIEnv* env;
    jobject* jo;
    template <class T>
    TmpJOBJ(JNIEnv* e, T* v) : env(e), jo((jobject*)v) { }
    TmpJOBJ(JNIEnv* e, jobject& v) : env(e), jo(&v) { }
    ~TmpJOBJ() { if (*jo) { env->DeleteLocalRef(*jo); *jo = 0; } }
};
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* /*ajvm*/, void* /*reserved*/) {
    return JNI_VERSION_1_6;
}
%}
#endif