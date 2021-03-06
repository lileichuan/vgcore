﻿//! \file gicoreviewimpl.h
//! \brief 定义GiCoreView实现类 GiCoreViewImpl
// Copyright (c) 2012-2014, https://github.com/rhcad/touchvg

#ifndef TOUCHVG_CORE_VIEWIMPL_H
#define TOUCHVG_CORE_VIEWIMPL_H

#include "gicoreviewdata.h"
#include "GcShapeDoc.h"
#include "GcMagnifierView.h"
#include "mgcmdmgr.h"
#include "mgcmdmgrfactory.h"
#include "cmdsubject.h"
#include "mgjsonstorage.h"
#include "mgstorage.h"
#include "girecordshape.h"
#include "mgshapet.h"
#include "cmdbasic.h"
#include "mglayer.h"
#include "mglog.h"
#include <map>

#define CALL_VIEW(func) if (curview) curview->func
#define CALL_VIEW2(func, v) curview ? curview->func : v

//! 供Java等语言用的 MgShape 实现类
class MgShapeExt : public MgShape
{
public:
    MgBaseShape* _shape;
    GiContext   _context;
    int         _id;
    MgShapes*   _parent;
    int         _tag;
    volatile long _refcount;
    
    MgShapeExt(MgBaseShape* shape)
    : _shape(shape), _id(0), _parent(NULL), _tag(0), _refcount(1) {
    }
    virtual ~MgShapeExt() { _shape->release(); }
    
    const GiContext& context() const { return _context; }
    void setContext(const GiContext& ctx, int mask) { _context.copy(ctx, mask); }
    MgBaseShape* shape() { return _shape; }
    const MgBaseShape* shapec() const { return _shape; }
    int getType() const { return 0x20000 | _shape->getType(); }
    void release() { if (giAtomicDecrement(&_refcount) == 0) delete this; }
    void addRef() { giAtomicIncrement(&_refcount); }
    int getTag() const { return _tag; }
    void setTag(int tag) { _tag = tag; }
    int getID() const { return _id; }
    MgShapes* getParent() const { return _parent; }
    
    MgObject* clone() const {
        MgShapeExt *p = new MgShapeExt(_shape->cloneShape());
        p->copy(*this);
        return p;
    }
    void setParent(MgShapes* p, int sid) {
        _parent = p;
        _id = sid;
        shape()->setOwner(this);
    }
};

//! GiCoreView实现类
class GiCoreViewImpl : public GiCoreViewData, public MgShapeFactory
{
public:
    static float    _factor;
    GcShapeDoc*     _gcdoc;
    MgCmdManager*   _cmds;
    GcBaseView*     curview;
    long            refcount;
    MgMotion        _motion;
    int             gestureHandler;
    MgJsonStorage   defaultStorage;
    
    long            regenPending;
    long            appendPending;
    long            redrawPending;
    volatile long   changeCount;
    volatile long   drawCount;
    
    std::map<int, MgShape* (*)()>   _shapeCreators;
    
    GiGraphics*     gsBuf[20];
    volatile long   gsUsed[20];
    volatile long   stopping;
    
public:
    GiCoreViewImpl(GiCoreView* owner, bool useCmds = true);
    ~GiCoreViewImpl();
    
    void submitBackXform() { CALL_VIEW(submitBackXform()); }
    
    MgMotion* motion() { return &_motion; }
    MgCmdManager* cmds() const { return _cmds; }
    GcShapeDoc* document() const { return _gcdoc; }
    MgShapeDoc* doc() const { return backDoc; }
    MgShapes* shapes() const { return backDoc->getCurrentShapes(); }
    GiContext* context() const { return backDoc->context(); }
    GiTransform* xform() const { return CALL_VIEW2(xform(), NULL); }
    Matrix2d& modelTransform() const { return backDoc->modelTransform(); }
    
    int getNewShapeID() { return _cmds->getNewShapeID(); }
    void setNewShapeID(int sid) { _cmds->setNewShapeID(sid); }
    CmdSubject* getCmdSubject() { return cmds()->getCmdSubject(); }
    MgSelection* getSelection() { return cmds()->getSelection(); }
    MgShapeFactory* getShapeFactory() { return this; }
    MgSnap* getSnap() { return _cmds->getSnap(); }
    MgActionDispatcher* getAction() {
        return _cmds->getActionDispatcher(); }
    
    bool registerCommand(const char* name, MgCommand* (*creator)()) {
        return _cmds->registerCommand(name, creator); }
    bool toSelectCommand() { return _cmds->setCommand(&_motion, "select", NULL); }
    const char* getCommandName() { return _cmds->getCommandName(); }
    MgCommand* getCommand() { return _cmds ? _cmds->getCommand() : NULL; }
    MgCommand* findCommand(const char* name) {
        return _cmds->findCommand(name); }
    bool setCommand(const char* name) { return _cmds->setCommand(&_motion, name, NULL); }
    bool setCurrentShapes(MgShapes* shapes) {
        return doc()->setCurrentShapes(shapes); }
    bool isReadOnly() const { return doc()->isReadOnly() || doc()->getCurrentLayer()->isLocked(); }
    bool isCommand(const char* name) { return name && strcmp(getCommandName(), name) == 0; }
    
    bool shapeWillAdded(MgShape* shape) {
        return !cmds() || getCmdSubject()->onShapeWillAdded(motion(), shape); }
    bool shapeWillDeleted(const MgShape* shape) {
        return !cmds() || getCmdSubject()->onShapeWillDeleted(motion(), shape); }
    bool shapeCanRotated(const MgShape* shape) {
        return !cmds() || getCmdSubject()->onShapeCanRotated(motion(), shape); }
    bool shapeCanTransform(const MgShape* shape) {
        return !cmds() || getCmdSubject()->onShapeCanTransform(motion(), shape); }
    bool shapeCanUnlock(const MgShape* shape) {
        return !cmds() || getCmdSubject()->onShapeCanUnlock(motion(), shape); }
    bool shapeCanUngroup(const MgShape* shape) {
        return !cmds() || getCmdSubject()->onShapeCanUngroup(motion(), shape); }
    void shapeMoved(MgShape* shape, int segment) {
        getCmdSubject()->onShapeMoved(motion(), shape, segment); }
    bool shapeWillChanged(MgShape* shape, const MgShape* oldsp) {
        return getCmdSubject()->onShapeWillChanged(motion(), shape, oldsp); }
    
    void commandChanged() {
        CALL_VIEW(deviceView()->commandChanged());
    }
    void selectionChanged() {
        CALL_VIEW(deviceView()->selectionChanged());
    }
    void dynamicChanged() {
        CALL_VIEW(deviceView()->dynamicChanged());
    }
    bool shapeClicked(int sid, int tag, float x, float y) {
        return CALL_VIEW2(deviceView()->shapeClicked(sid, tag, x, y), false);
    }
    void showMessage(const char* text) {
        CALL_VIEW(deviceView()->showMessage(text));
    }
    void getLocalizedString(const char* name, MgStringCallback* result) {
        CALL_VIEW(deviceView()->getLocalizedString(name, result));
    }
    
    bool removeShape(const MgShape* shape) {
        hideContextActions();
        bool ret = (shape && shape->getParent()
                    && shape->getParent()->findShape(shape->getID()) == shape
                    && !shape->shapec()->getFlag(kMgShapeLocked));
        if (ret) {
            int sid = shape->getID();
            getCmdSubject()->onShapeDeleted(motion(), shape);
            ret = shape->getParent()->removeShape(shape->getID());
            CALL_VIEW(deviceView()->shapeDeleted(sid));
        }
        return ret;
    }
    
    bool useFinger() {
        return CALL_VIEW2(deviceView()->useFinger(), true);
    }
    
    bool isContextActionsVisible() {
        return CALL_VIEW2(deviceView()->isContextActionsVisible(), false);
    }
    
    void hideContextActions() {
        CALL_VIEW(deviceView()->hideContextActions());
    }
    
    bool showContextActions(int /*selState*/, const int* actions,
                            const Box2d& selbox, const MgShape*)
    {
        int n = 0;
        for (; actions && actions[n] > 0; n++) {}
        
        if (!curview || (n > 0 && motion()->pressDrag && isContextActionsVisible())) {
            return false;
        }
        mgvector<int> arr(actions, n);
        mgvector<float> pos(2 * n);
        calcContextButtonPosition(pos, n, selbox);
        return CALL_VIEW2(deviceView()->showContextActions(arr, pos,
                selbox.xmin, selbox.ymin, selbox.width(), selbox.height()), false);
    }
    
    void shapeAdded(const MgShape* sp) {
        regenAppend(sp->getID());
        getCmdSubject()->onShapeAdded(motion(), sp);
    }
    
    void redraw(bool changed = true) {
        if (redrawPending >= 0) {
            redrawPending += changed ? 100 : 1;
        }
        else {
            CALL_VIEW(deviceView()->redraw(changed));
        }
    }
    
    void regenAll(bool changed) {
        bool apply = regenPending || appendPending;
        bool zooming = CALL_VIEW2(isZooming(), false);
        
        if (regenPending >= 0) {
            regenPending += changed ? 100 : 1;
        }
        if (apply) {
            CALL_VIEW(deviceView()->regenAll(changed));
            if (changed) {
                for (int i = 0; i < _gcdoc->getViewCount(); i++) {
                    if (_gcdoc->getView(i) != curview && !zooming)
                        _gcdoc->getView(i)->deviceView()->regenAll(changed);
                }
                CALL_VIEW(deviceView()->contentChanged());
            }
            else {
                for (int i = 0; i < _gcdoc->getViewCount(); i++) {
                    if (_gcdoc->getView(i) != curview && !zooming)
                        _gcdoc->getView(i)->deviceView()->redraw(changed);
                }
            }
        }
    }
    
    void regenAppend(int sid, long playh = 0) {
        bool apply = regenPending || appendPending;
        
        if (appendPending >= 0 && sid) {
            if (appendPending == 0 || appendPending == sid) {
                appendPending = sid;
            }
            else if (appendPending > 0 && appendPending != sid) {
                regenAll(true);
            }
        }
        sid = sid ? sid : (int)appendPending;
        if (apply && sid) {
            CALL_VIEW(deviceView()->regenAppend(sid, playh));
            for (int i = 0; i < _gcdoc->getViewCount(); i++) {
                if (_gcdoc->getView(i) != curview)
                    _gcdoc->getView(i)->deviceView()->regenAppend(sid, playh);
            }
            CALL_VIEW(deviceView()->contentChanged());
        }
    }
    
    bool setView(GcBaseView* view) {
        if (curview != view) {
            GcBaseView* oldview = curview;
            curview = view;
            CALL_VIEW(deviceView()->viewChanged(oldview ? oldview->deviceView() : NULL));
        }
        return !!view;
    }
    
    bool gestureToCommand();
    void submitDynamicShapes(GcBaseView* v);
    
private:
    void registerShape(int type, MgShape* (*creator)()) {
        type = type & 0xFFFF;
        if (creator) {
            _shapeCreators[type] = creator;
        }
        else {
            _shapeCreators.erase(type);
        }
    }
    MgShape* createShape(int type) {
        std::map<int, MgShape* (*)()>::const_iterator it = _shapeCreators.find(type & 0xFFFF);
        if (it != _shapeCreators.end()) {
            return (it->second)();
        }
        
        MgBaseShape* sp = getCmdSubject()->createShape(motion(), type);
        if (sp) {
            return new MgShapeExt(sp);
        }
        
        return NULL;
    }
    
private:
    void calcContextButtonPosition(mgvector<float>& pos, int n, const Box2d& box);
    Box2d calcButtonPosition(mgvector<float>& pos, int n, const Box2d& selbox);
    Vector2d moveActionsInView(Box2d& rect, float btnHalfW);
};

// DrawLocker
//

class DrawLocker
{
    GiCoreViewImpl* _impl;
public:
    DrawLocker(GiCoreViewImpl* impl) : _impl(NULL) {
        if (impl->regenPending < 0
            && impl->appendPending < 0
            && impl->redrawPending < 0)
        {
            _impl = impl;
            _impl->regenPending = 0;
            _impl->appendPending = 0;
            _impl->redrawPending = 0;
        }
    }
    
    ~DrawLocker() {
        if (!_impl) {
            return;
        }
        long regenPending = _impl->regenPending;
        long appendPending = _impl->appendPending;
        long redrawPending = _impl->redrawPending;
        
        _impl->regenPending = -1;
        _impl->appendPending = -1;
        _impl->redrawPending = -1;
        
        if (regenPending > 0) {
            _impl->regenAll(regenPending >= 100);
        }
        else if (appendPending > 0) {
            _impl->regenAppend((int)appendPending);
        }
        else if (redrawPending > 0) {
            _impl->redraw(redrawPending >= 100);
        }
    }
};

#endif // TOUCHVG_CORE_VIEWIMPL_H
