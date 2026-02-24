//
// Copyright (c) 1993-2023 Robert McNeel & Associates. All rights reserved.
// Rhinoceros is a registered trademark of Robert McNeel & Associates.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// ALL IMPLIED WARRANTIES OF FITNESS FOR ANY PARTICULAR PURPOSE AND OF
// MERCHANTABILITY ARE HEREBY DISCLAIMED.
//				
// For complete Rhino SDK copyright information see <http://www.rhino3d.com/developer>.
//
////////////////////////////////////////////////////////////////

#pragma once

enum class EnvironmentBackgroundProjection : int
{
	planar = 0,
	spherical = 1,
	emap = 2,
	box = 3,
	lightprobe = 4,
	cubemap = 5,
	vertical_cross_cubemap = 6,
	horizontal_cross_cubemap = 7,
	hemispherical = 8,
	none = -1
};

enum class RhinoDashStyle : unsigned int
{
	Solid,
	Dash
};

enum ECullFaceMode : unsigned int
{
	CFM_DRAW_FRONT_AND_BACK = 0,
	CFM_DRAW_FRONT_FACES = 1,
	CFM_DRAW_BACK_FACES = 2,
};

/////////////////////////////////////////////////////////////////////////////
// Points helper types and classes used to describe and draw points...
enum ERhinoPointStyle : unsigned int
{
	RPS_SQUARE = 50, // same as VARIABLE_DOT
	RPS_CIRCLE = 51, // same as ROUND_DOT
	RPS_TRIANGLE = 2,
	RPS_HEART = 3,
	RPS_CHEVRON = 4,
	RPS_CLOVER = 5,
	RPS_TAG = 6,
	RPS_X = 7,
	RPS_ASTERISK = 8,
	RPS_PIN = 9,
	RPS_ARROW_TAIL_OFF_POINT = 10,
	RPS_ARROW_TIP_OFF_POINT = 11,

	// "DOT's" are solid filled, single color entities...
	RPS_VARIABLE_DOT    = 50, // varying radius
	RPS_ROUND_DOT       = 51,
	
	// "POINT's" are border filled color with white centers, 2 color entities...
	// Note: The radius can vary on all of these...
	RPS_CONTROL_POINT   = 100, // Rhino's standard CV object...
	RPS_ACTIVE_POINT    = 101, // Rhino's standard active CV object...
	RPS_ROUND_CONTROL_POINT = 102,
	RPS_ROUND_ACTIVE_POINT = 103,
};

enum class RhinoLineCapStyle : int
{
	None   = 0,
	Round  = 1,
	Flat   = 2,
	Square = 3
};

enum class RhinoLineJoinStyle : int
{
	None    = 0,
	Round   = 1,
	Miter   = 2,
	Bevel   = 3
};

struct CRhCurveAttributes
{
	ON_Color           m_Color;
	int                m_nThickness;
	UINT               m_nPattern;
	RhinoLineCapStyle  m_eEndCapStyle;
	RhinoLineJoinStyle m_eJoinStyle;
	bool               m_bClip;
};

/////////////////////////////////////////////////////////////////////////////
// Lines helper class used to describe and draw large chunks of lines
class CLinesDescriptor
{
public:
	CLinesDescriptor(const ON_3dPoint*  points, int count) :
		m_Points(points),
		m_Count(count)
	{
		m_Color = ON_Color::Black;
	}

public:
	const ON_3dPoint*   m_Points;    // consecutive points that make up
																	 // individual line segment pairs.
																	 // (moveto, lineto), (moveto, lineto),...

	int                 m_Count;     // number of points in m_Points
	int                 m_Thickness = 1;
	UINT                m_Pattern = 0xFFFFFFFF;
	ON_Color            m_Color;
};
