/***************************************************************************
 *   Copyright (C) 2009-2015 by Veselin Georgiev, Slavomir Kaslev et al    *
 *   admin@raytracing-bg.net                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/**
 * @File constants.h
 * @Brief A few useful constants
 */
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__


#define VFB_MAX_SIZE 1920
#define RESX 640
#define RESY 480
#define PI 3.141592653589793238
#define INF 1e99

// large `float' number:
#define LARGE_FLOAT 1e17f

// large `double' number:
#define LARGE_DOUBLE 1e120

// Constants for the building of KD trees:
#define MAX_TREE_DEPTH 64
#define TRIANGLES_PER_LEAF 20

#endif // __CONSTANTS_H__
