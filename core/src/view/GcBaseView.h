﻿//! \file GcBaseView.h
//! \brief 定义内核视图基类 GcBaseView
// Copyright (c) 2012-2013, https://github.com/rhcad/touchvg

#ifndef TOUCHVG_CORE_BASEVIEW_H
#define TOUCHVG_CORE_BASEVIEW_H

#include "gigesture.h"
#include "mgcmd.h"
#include "mgshapedoc.h"

class GiView;

//! 内核视图基类
/*! \ingroup CORE_VIEW
 */
class GcBaseView
{
public:
    GcBaseView(MgView* mgview, GiView *view);
    virtual ~GcBaseView();
    
    GiView* deviceView() { return _view; }                          //!< 返回回调视图对象
    MgView* cmdView() { return _mgview; }
    bool isZooming() { return _zooming; }
    void setZoomEnabled(bool enabled) { _zoomEnabled = enabled; }
    
    void submitBackXform() { _gsFront.copy(_gsBack); }              //!< 应用后端坐标系对象到前端
    void copyGs(GiGraphics* gs) { gs->copy(_gsBack); }              //!< 复制坐标系参数
    
    GiGraphics* frontGraph() { return &_gsFront; }                  //!< 得到前端图形显示对象
    GiTransform* xform() { return &_gsBack._xf(); }                 //!< 得到后端坐标系对象
    GiGraphics* graph() { return &_gsBack; }                        //!< 得到后端图形显示对象
    virtual void onSize(int dpi, int w, int h);                     //!< 设置视图的宽高
    
    virtual bool onGesture(const MgMotion& motion);                 //!< 传递单指触摸手势消息
    virtual bool twoFingersMove(const MgMotion& motion);            //!< 传递双指移动手势(可放缩旋转)

private:
    MgView*     _mgview;
    GiView*     _view;
    GiGraphics  _gsFront;
    GiGraphics  _gsBack;
    Point2d     _lastCenter;
    float       _lastScale;
    bool        _zooming;
    bool        _zoomEnabled;
};

#endif // TOUCHVG_CORE_BASEVIEW_H
