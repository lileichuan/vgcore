﻿// gipath.cpp: 实现路径类 GiPath
// Copyright (c) 2004-2013, Zhang Yungui
// License: LGPL, https://github.com/rhcad/touchvg

#include "gipath.h"
#include "mgcurv.h"
#include <vector>

// 返回STL数组(vector)变量的元素个数
template<class T> inline static int getSize(T& arr)
{
    return static_cast<int>(arr.size());
}

//! GiPath的内部数据类
class GiPathImpl
{
public:
    std::vector<Point2d>    points;         //!< 每个节点的坐标
    std::vector<char>       types;          //!< 每个节点的类型, kGiLineTo 等
    int                     beginIndex;     //!< 新图形的起始节点(即MOVETO节点)的序号
};

GiPath::GiPath()
{
    m_data = new GiPathImpl();
    m_data->beginIndex = -1;
}

GiPath::GiPath(const GiPath& src)
{
    m_data = new GiPathImpl();

    unsigned count = (unsigned)src.m_data->points.size();
    m_data->points.reserve(count);
    m_data->types.reserve(count);
    for (unsigned i = 0; i < count; i++) {
        m_data->points.push_back(src.m_data->points[i]);
        m_data->types.push_back(src.m_data->types[i]);
    }
    m_data->beginIndex = src.m_data->beginIndex;
}

GiPath::GiPath(int count, const Point2d* points, const char* types)
{
    m_data = new GiPathImpl();
    m_data->beginIndex = -1;

    if (count > 0 && points && types) {
        m_data->points.reserve(count);
        m_data->types.reserve(count);
        for (int i = 0; i < count; i++) {
            m_data->points.push_back(points[i]);
            m_data->types.push_back(types[i]);
        }
    }
}

GiPath::~GiPath()
{
    delete m_data;
}

GiPath& GiPath::copy(const GiPath& src)
{
    if (this != &src) {
        clear();
        unsigned count = (unsigned)src.m_data->points.size();
        m_data->points.reserve(count);
        m_data->types.reserve(count);
        for (unsigned i = 0; i < count; i++) {
            m_data->points.push_back(src.m_data->points[i]);
            m_data->types.push_back(src.m_data->types[i]);
        }
        m_data->beginIndex = src.m_data->beginIndex;
    }
    return *this;
}

GiPath& GiPath::append(const GiPath& src)
{
    if (this != &src && src.getCount() > 1 && getCount() > 1) {
        size_t i = 0;
        
        if (src.getNodeType(0) == kGiMoveTo
            && !(m_data->types.back() & kGiCloseFigure)
            && getEndPoint() == src.getPoint(0)) {
            i++;    // skip moveto
        }
        for (; i < m_data->types.size(); i++) {
            m_data->points.push_back(src.m_data->points[i]);
            m_data->types.push_back(src.m_data->types[i]);
        }
    }
    return *this;
}

void GiPath::setPath(int count, const Point2d* points, const char* types)
{
    if (getCount() != count) {
        clear();
        if (count > 0 && points && types) {
            m_data->points.reserve(count);
            m_data->types.reserve(count);
            for (int i = 0; i < count; i++) {
                m_data->points.push_back(points[i]);
                m_data->types.push_back(types[i]);
            }
        }
    } else {
        for (int i = 0; i < count; i++) {
            m_data->points[i] = points[i];
            m_data->types[i] = types[i];
        }
    }
}

void GiPath::setPath(int count, const Point2d* points, const int* types)
{
    if (getCount() != count) {
        clear();
        if (count > 0 && points && types) {
            m_data->points.reserve(count);
            m_data->types.reserve(count);
            for (int i = 0; i < count; i++) {
                m_data->points.push_back(points[i]);
                m_data->types.push_back((char)types[i]);
            }
        }
    } else {
        for (int i = 0; i < count; i++) {
            m_data->points[i] = points[i];
            m_data->types[i] = (char)types[i];
        }
    }
}

int GiPath::getCount() const
{
    return getSize(m_data->points);
}

Point2d GiPath::getStartPoint() const
{
    return m_data->points.empty() ? Point2d() : m_data->points.front();
}

Vector2d GiPath::getStartTangent() const
{
    return (m_data->points.size() < 2 ? Vector2d() :
            m_data->points[1] - m_data->points.front());
}

Point2d GiPath::getEndPoint() const
{
    return m_data->points.empty() ? Point2d() : m_data->points.back();
}

Vector2d GiPath::getEndTangent() const
{
    return (m_data->points.size() < 2 ? Vector2d() :
            m_data->points.back() - m_data->points[m_data->points.size() - 2]);
}

const Point2d* GiPath::getPoints() const
{
    return m_data->points.empty() ? (Point2d*)0 : &m_data->points.front();
}

const char* GiPath::getTypes() const
{
    return m_data->types.empty() ? (char*)0 : &m_data->types.front();
}

int GiPath::getNodeType(int index) const
{
    return index >= 0 && index < getCount() ? (int)m_data->types[index] : 0;
}

Point2d GiPath::getPoint(int index) const
{
    return index >= 0 && index < getCount() ? m_data->points[index] : Point2d();
}

void GiPath::setPoint(int index, const Point2d& pt)
{
    if (index >= 0 && index < getCount())
        m_data->points[index] = pt;
}

void GiPath::clear()
{
    m_data->points.clear();
    m_data->types.clear();
    m_data->beginIndex = -1;
}

void GiPath::transform(const Matrix2d& mat)
{
    for (unsigned i = 0; i < m_data->points.size(); i++) {
        m_data->points[i] *= mat;
    }
}

void GiPath::startFigure()
{
    m_data->beginIndex = -1;
}

bool GiPath::moveTo(const Point2d& point, bool rel)
{
    m_data->points.push_back(rel ? point + getEndPoint() : point);
    m_data->types.push_back(kGiMoveTo);
    m_data->beginIndex = getSize(m_data->points) - 1;

    return true;
}

bool GiPath::lineTo(const Point2d& point, bool rel)
{
    bool ret = (m_data->beginIndex >= 0);
    if (ret) {
        m_data->points.push_back(rel ? point + getEndPoint() : point);
        m_data->types.push_back(kGiLineTo);
    }

    return ret;
}

bool GiPath::horzTo(float x, bool rel)
{
    Point2d pt(getEndPoint());
    bool ret = (m_data->beginIndex >= 0);
    
    if (ret) {
        pt.x = rel ? pt.x + x : x;
        m_data->points.push_back(pt);
        m_data->types.push_back(kGiLineTo);
    }
    
    return ret;
}

bool GiPath::vertTo(float y, bool rel)
{
    Point2d pt(getEndPoint());
    bool ret = (m_data->beginIndex >= 0);
    
    if (ret) {
        pt.y = rel ? pt.y + y : y;
        m_data->points.push_back(pt);
        m_data->types.push_back(kGiLineTo);
    }
    
    return ret;
}

bool GiPath::linesTo(int count, const Point2d* points, bool rel)
{
    bool ret = (m_data->beginIndex >= 0 && count > 0 && points);
    Point2d lastpt(getEndPoint());
    
    if (ret) {
        for (int i = 0; i < count; i++) {
            m_data->points.push_back(rel ? points[i] + lastpt : points[i]);
            m_data->types.push_back(kGiLineTo);
        }
    }

    return ret;
}

bool GiPath::beziersTo(int count, const Point2d* points, bool reverse, bool rel)
{
    bool ret = (m_data->beginIndex >= 0 && count > 0 && points
        && (count % 3) == 0);
    Point2d lastpt(getEndPoint());
    
    if (ret && reverse) {
        for (int i = count - 1; i >= 0; i--) {
            m_data->points.push_back(rel ? points[i] + lastpt : points[i]);
            m_data->types.push_back(kGiBezierTo);
        }
    }
    else if (ret) {
        for (int i = 0; i < count; i++) {
            m_data->points.push_back(rel ? points[i] + lastpt : points[i]);
            m_data->types.push_back(kGiBezierTo);
        }
    }

    return ret;
}

bool GiPath::bezierTo(const Point2d& cp1, const Point2d& cp2, const Point2d& end, bool rel)
{
    Point2d lastpt(getEndPoint());
    
    m_data->points.push_back(rel ? cp1 + lastpt : cp1);
    m_data->points.push_back(rel ? cp2 + lastpt : cp2);
    m_data->points.push_back(rel ? end + lastpt : end);
    for (int i = 0; i < 3; i++)
        m_data->types.push_back(kGiBezierTo);
    
    return true;
}

bool GiPath::smoothBezierTo(const Point2d& cp2, const Point2d& end, bool rel)
{
    Point2d lastpt(getEndPoint());
    Point2d cp1(m_data->points.size() > 1 ? 2 * lastpt -
                m_data->points[m_data->points.size() - 2].asVector() : lastpt);
    
    m_data->points.push_back(cp1);
    m_data->points.push_back(rel ? cp2 + lastpt : cp2);
    m_data->points.push_back(rel ? end + lastpt : end);
    for (int i = 0; i < 3; i++)
        m_data->types.push_back(kGiBezierTo);
    
    return true;
}

bool GiPath::quadsTo(int count, const Point2d* points, bool rel)
{
    bool ret = (m_data->beginIndex >= 0 && count > 0 && points
                && (count % 2) == 0);
    Point2d lastpt(getEndPoint());
    
    if (ret) {
        for (int i = 0; i < count; i++) {
            m_data->points.push_back(rel ? points[i] + lastpt : points[i]);
            m_data->types.push_back(kGiQuadTo);
        }
    }
    
    return ret;
}

bool GiPath::quadTo(const Point2d& cp, const Point2d& end, bool rel)
{
    Point2d lastpt(getEndPoint());
    
    m_data->points.push_back(rel ? cp + lastpt : cp);
    m_data->points.push_back(rel ? end + lastpt : end);
    m_data->types.push_back(kGiQuadTo);
    m_data->types.push_back(kGiQuadTo);
    return true;
}

bool GiPath::smoothQuadTo(const Point2d& end, bool rel)
{
    Point2d lastpt(getEndPoint());
    Point2d cp(m_data->points.size() > 1 ? 2 * lastpt -
               m_data->points[m_data->points.size() - 2].asVector() : lastpt);
    
    m_data->points.push_back(cp);
    m_data->points.push_back(rel ? end + lastpt : end);
    m_data->types.push_back(kGiQuadTo);
    m_data->types.push_back(kGiQuadTo);
    return true;
}

bool GiPath::arcTo(const Point2d& point, bool rel)
{
    bool ret = false;

    if (m_data->beginIndex >= 0 
        && getSize(m_data->points) >= m_data->beginIndex + 2
        && m_data->points.size() == m_data->types.size())
    {
        Point2d start = m_data->points[m_data->points.size() - 1];
        Vector2d tanv = start - m_data->points[m_data->points.size() - 2];
        Point2d center;
        float radius, startAngle, sweepAngle;

        if (mgcurv::arcTan(start, rel ? point + getEndPoint() : point,
                           tanv, center, radius, &startAngle, &sweepAngle)) {
            Point2d pts[16];
            int n = mgcurv::arcToBezier(pts, center, radius, radius, 
                startAngle, sweepAngle);
            if (n >= 4) {
                ret = true;
                for (int i = 0; i < n; i++) {
                    m_data->points.push_back(pts[i]);
                    m_data->types.push_back(kGiBezierTo);
                }
            }
        }
    }

    return ret;
}

bool GiPath::arcTo(const Point2d& point, const Point2d& end, bool rel)
{
    bool ret = false;
    Point2d lastpt(getEndPoint());

    if (m_data->beginIndex >= 0 
        && getSize(m_data->points) >= m_data->beginIndex + 1
        && m_data->points.size() == m_data->types.size())
    {
        Point2d start = m_data->points[m_data->points.size() - 1];
        Point2d center;
        float radius, startAngle, sweepAngle;

        if (mgcurv::arc3P(start, rel ? point + lastpt : point,
                          rel ? end + lastpt : end,
                          center, radius, &startAngle, &sweepAngle)) {
            Point2d pts[16];
            int n = mgcurv::arcToBezier(pts, center, radius, radius, 
                startAngle, sweepAngle);
            if (n >= 4) {
                ret = true;
                for (int i = 0; i < n; i++) {
                    m_data->points.push_back(pts[i]);
                    m_data->types.push_back(kGiBezierTo);
                }
            }
        }
    }

    return ret;
}

bool GiPath::closeFigure()
{
    bool ret = false;

    if (m_data->beginIndex >= 0 
        && getSize(m_data->points) >= m_data->beginIndex + 3
        && m_data->points.size() == m_data->types.size())
    {
        char type = m_data->types[m_data->points.size() - 1];
        if (type == kGiLineTo || type == kGiBezierTo || type == kGiQuadTo) {
            m_data->types[m_data->points.size() - 1] |= kGiCloseFigure;
            m_data->beginIndex = -1;
            ret = true;
        }
    }

    return ret;
}

static int AngleToBezier(Point2d* pts, float radius)
{
    const Vector2d vec1 (pts[1] - pts[0]);      // 第一条边
    const Vector2d vec2 (pts[2] - pts[1]);      // 第二条边

    const float dHalfAngle = 0.5f * fabsf(vec1.angleTo2(vec2));  // 夹角的一半
    if (dHalfAngle < 1e-4f || fabsf(dHalfAngle - _M_PI_2) < 1e-4f)  // 两条边平行
        return 0;

    const float dDist1 = 0.5f * vec1.length();
    const float dDist2 = 0.5f * vec2.length();
    float dArc = radius / tan(dHalfAngle);    // 圆弧在边上的投影长度
    if (dArc > dDist1 || dArc > dDist2) {
        float dArcOld = dArc;
        dArc = mgMin(dDist1, dDist2);
        if (dArc < dArcOld * 0.5f)
            return 3;
    }

    int count = 0;
    Point2d ptCenter, ptStart, ptEnd;
    float startAngle, sweepAngle;

    ptStart = pts[1].rulerPoint(pts[0], dArc, 0);
    ptEnd = pts[1].rulerPoint(pts[2], dArc, 0);
    if (mgcurv::arcTan(ptStart, ptEnd, pts[1] - ptStart, 
        ptCenter, radius, &startAngle, &sweepAngle))
    {
        count = mgcurv::arcToBezier(
            pts, ptCenter, radius, radius, startAngle, sweepAngle);
    }

    return count;
}

bool GiPath::genericRoundLines(int count, const Point2d* points, 
                               float radius, bool closed)
{
    clear();

    if (count < 3 || !points || radius < _MGZERO)
        return false;

    Point2d ptsBzr[16];
    int nBzrCnt;

    if (closed) {
        ptsBzr[0] = points[count - 1];
        ptsBzr[1] = points[0];
        ptsBzr[2] = points[1];
        nBzrCnt = AngleToBezier(ptsBzr, radius);
        if (nBzrCnt < 4) {
            this->moveTo(points[0]);
        }
        else {
            this->moveTo(ptsBzr[0]);
            this->beziersTo(nBzrCnt - 1, ptsBzr + 1);
        }
    }
    else {
        this->moveTo(points[0]);
    }

    for (int i = 1; i < (closed ? count : count - 1); i++) {
        ptsBzr[0] = points[i - 1];
        ptsBzr[1] = points[i];
        ptsBzr[2] = points[(i + 1) % count];
        nBzrCnt = AngleToBezier(ptsBzr, radius);
        if (nBzrCnt < 4) {
            this->lineTo(points[i]);
        }
        else {
            this->lineTo(ptsBzr[0]);
            this->beziersTo(nBzrCnt - 1, ptsBzr + 1);
        }
    }

    if (closed)
        this->closeFigure();
    else
        this->lineTo(points[count - 1]);

    return true;
}
