/*
	Copyright (C) 2006 yopyop
	Copyright (C) 2006-2007 shash
	Copyright (C) 2008-2018 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "OGLRender_3_2.h"

#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "utils/bits.h"
#include "common.h"
#include "debug.h"
#include "NDSSystem.h"

//------------------------------------------------------------

// Basic Functions
OGLEXT(PFNGLGETSTRINGIPROC, glGetStringi) // Core in v3.0
OGLEXT(PFNGLCLEARBUFFERFVPROC, glClearBufferfv) // Core in v3.0
OGLEXT(PFNGLCLEARBUFFERFIPROC, glClearBufferfi) // Core in v3.0

// Shaders
OGLEXT(PFNGLBINDFRAGDATALOCATIONPROC, glBindFragDataLocation) // Core in v3.0

// Buffer Objects
OGLEXT(PFNGLMAPBUFFERRANGEPROC, glMapBufferRange) // Core in v3.0

// FBO
OGLEXT(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) // Core in v3.0
OGLEXT(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) // Core in v3.0
OGLEXT(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer) // Core in v3.0
OGLEXT(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) // Core in v3.0
OGLEXT(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) // Core in v3.0
OGLEXT(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) // Core in v3.0
OGLEXT(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer) // Core in v3.0
OGLEXT(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers) // Core in v3.0
OGLEXT(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer) // Core in v3.0
OGLEXT(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage) // Core in v3.0
OGLEXT(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample) // Core in v3.0
OGLEXT(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers) // Core in v3.0
OGLEXT(PFNGLTEXIMAGE2DMULTISAMPLEPROC, glTexImage2DMultisample) // Core in v3.2

// UBO
OGLEXT(PFNGLGETUNIFORMBLOCKINDEXPROC, glGetUniformBlockIndex) // Core in v3.1
OGLEXT(PFNGLUNIFORMBLOCKBINDINGPROC, glUniformBlockBinding) // Core in v3.1
OGLEXT(PFNGLBINDBUFFERBASEPROC, glBindBufferBase) // Core in v3.0
OGLEXT(PFNGLGETACTIVEUNIFORMBLOCKIVPROC, glGetActiveUniformBlockiv) // Core in v3.1

// TBO
OGLEXT(PFNGLTEXBUFFERPROC, glTexBuffer) // Core in v3.1

void OGLLoadEntryPoints_3_2()
{
	// Basic Functions
	INITOGLEXT(PFNGLGETSTRINGIPROC, glGetStringi)
	INITOGLEXT(PFNGLCLEARBUFFERFVPROC, glClearBufferfv)
	INITOGLEXT(PFNGLCLEARBUFFERFIPROC, glClearBufferfi)
	
	// Shaders
	INITOGLEXT(PFNGLBINDFRAGDATALOCATIONPROC, glBindFragDataLocation)
	
	// Buffer Objects
	INITOGLEXT(PFNGLMAPBUFFERRANGEPROC, glMapBufferRange)
	
	// FBO
	INITOGLEXT(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) // Promote to core version
	INITOGLEXT(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) // Promote to core version
	INITOGLEXT(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer) // Promote to core version
	INITOGLEXT(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) // Promote to core version
	INITOGLEXT(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) // Promote to core version
	INITOGLEXT(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) // Promote to core version
	INITOGLEXT(PFNGLBLITFRAMEBUFFERPROC, glBlitFramebuffer) // Promote to core version
	INITOGLEXT(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers) // Promote to core version
	INITOGLEXT(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer) // Promote to core version
	INITOGLEXT(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage) // Promote to core version
	INITOGLEXT(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC, glRenderbufferStorageMultisample) // Promote to core version
	INITOGLEXT(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers) // Promote to core version
	INITOGLEXT(PFNGLTEXIMAGE2DMULTISAMPLEPROC, glTexImage2DMultisample)
	
	// UBO
	INITOGLEXT(PFNGLGETUNIFORMBLOCKINDEXPROC, glGetUniformBlockIndex)
	INITOGLEXT(PFNGLUNIFORMBLOCKBINDINGPROC, glUniformBlockBinding)
	INITOGLEXT(PFNGLBINDBUFFERBASEPROC, glBindBufferBase)
	INITOGLEXT(PFNGLGETACTIVEUNIFORMBLOCKIVPROC, glGetActiveUniformBlockiv)
	
	// TBO
	INITOGLEXT(PFNGLTEXBUFFERPROC, glTexBuffer)
}

// Vertex shader for geometry, GLSL 1.50
static const char *GeometryVtxShader_150 = {"\
	#version 150 \n\
	\n\
	in vec4 inPosition; \n\
	in vec2 inTexCoord0; \n\
	in vec3 inColor; \n\
	\n\
	uniform usamplerBuffer PolyStates;\n\
	uniform int polyIndex;\n\
	\n\
	out vec4 vtxPosition; \n\
	out vec2 vtxTexCoord; \n\
	out vec4 vtxColor; \n\
	flat out uint polyEnableTexture;\n\
	flat out uint polyEnableFog;\n\
	flat out uint polyIsWireframe;\n\
	flat out uint polySetNewDepthForTranslucent;\n\
	flat out uint polyMode;\n\
	flat out uint polyID;\n\
	flat out uint texSingleBitAlpha;\n\
	\n\
	void main() \n\
	{ \n\
		uvec4 polyStateFlags = texelFetch(PolyStates, (polyIndex*3)+0);\n\
		uvec4 polyStateValues = texelFetch(PolyStates, (polyIndex*3)+1);\n\
		uvec4 polyStateTexParams = texelFetch(PolyStates, (polyIndex*3)+2);\n\
		\n\
		float polyAlpha = float(polyStateValues[0]) / 31.0;\n\
		vec2 polyTexScale = vec2(1.0 / float(8 << polyStateTexParams[0]), 1.0 / float(8 << polyStateTexParams[1]));\n\
		\n\
		polyEnableTexture = polyStateFlags[0];\n\
		polyEnableFog = polyStateFlags[1];\n\
		polyIsWireframe = polyStateFlags[2];\n\
		polySetNewDepthForTranslucent = polyStateFlags[3];\n\
		polyMode = polyStateValues[1];\n\
		polyID = polyStateValues[2];\n\
		texSingleBitAlpha = polyStateTexParams[2];\n\
		\n\
		mat2 texScaleMtx	= mat2(	vec2(polyTexScale.x,            0.0), \n\
									vec2(           0.0, polyTexScale.y)); \n\
		\n\
		vtxPosition = inPosition; \n\
		vtxTexCoord = texScaleMtx * inTexCoord0; \n\
		vtxColor = vec4(inColor * 4.0, polyAlpha); \n\
		\n\
		gl_Position = vtxPosition; \n\
	} \n\
"};

// Fragment shader for geometry, GLSL 1.50
static const char *GeometryFragShader_150 = {"\
	#version 150 \n\
	#define DEPTH_EQUALS_TEST_TOLERANCE 255.0\n\
	\n\
	in vec4 vtxPosition;\n\
	in vec2 vtxTexCoord;\n\
	in vec4 vtxColor;\n\
	flat in uint polyEnableTexture;\n\
	flat in uint polyEnableFog;\n\
	flat in uint polyIsWireframe;\n\
	flat in uint polySetNewDepthForTranslucent;\n\
	flat in uint polyMode;\n\
	flat in uint polyID;\n\
	flat in uint texSingleBitAlpha;\n\
	\n\
	layout (std140) uniform RenderStates\n\
	{\n\
		vec2 framebufferSize;\n\
		int toonShadingMode;\n\
		bool enableAlphaTest;\n\
		bool enableAntialiasing;\n\
		bool enableEdgeMarking;\n\
		bool enableFogAlphaOnly;\n\
		bool useWDepth;\n\
		int clearPolyID;\n\
		float clearDepth;\n\
		float alphaTestRef;\n\
		float fogOffset;\n\
		float fogStep;\n\
		float pad_0;\n\
		vec4 fogColor;\n\
		float fogDensity[32];\n\
		vec4 edgeColor[8];\n\
		vec4 toonColor[32];\n\
	} state;\n\
	\n\
	uniform sampler2D texRenderObject;\n\
	uniform usamplerBuffer PolyStates;\n\
	uniform bool texDrawOpaque;\n\
	uniform bool polyDrawShadow;\n\
	uniform int polyIndex;\n\
	uniform int polyDepthOffsetMode;\n\
	\n\
	out vec4 outFragColor;\n\
	out vec4 outPolyID;\n\
	out vec4 outFogAttributes;\n\
	\n\
	void main()\n\
	{\n\
		vec4 newFragColor = vec4(0.0, 0.0, 0.0, 0.0);\n\
		vec4 newPolyID = vec4(0.0, 0.0, 0.0, 0.0);\n\
		vec4 newFogAttributes = vec4(0.0, 0.0, 0.0, 0.0);\n\
		\n\
		float vertW = (vtxPosition.w == 0.0) ? 0.00000001 : vtxPosition.w;\n\
		float depthOffset = (polyDepthOffsetMode == 0) ? 0.0 : ((polyDepthOffsetMode == 1) ? -DEPTH_EQUALS_TEST_TOLERANCE : DEPTH_EQUALS_TEST_TOLERANCE);\n\
		// hack: when using z-depth, drop some LSBs so that the overworld map in Dragon Quest IV shows up correctly\n\
		float newFragDepthValue = (state.useWDepth) ? clamp( ( (vtxPosition.w * 4096.0) + depthOffset ) / 16777215.0, 0.0, 1.0 ) : clamp( ( (floor(((vtxPosition.z/vertW) * 0.5 + 0.5) * 4194303.0) * 4.0) + depthOffset ) / 16777215.0, 0.0, 1.0 );\n\
		\n\
		if ((polyMode != 3u) || polyDrawShadow)\n\
		{\n\
			vec4 mainTexColor = bool(polyEnableTexture) ? texture(texRenderObject, vtxTexCoord) : vec4(1.0, 1.0, 1.0, 1.0);\n\
			\n\
			if (bool(texSingleBitAlpha))\n\
			{\n\
				if (mainTexColor.a < 0.500)\n\
				{\n\
					mainTexColor.a = 0.0;\n\
				}\n\
				else\n\
				{\n\
					mainTexColor.rgb = mainTexColor.rgb / mainTexColor.a;\n\
					mainTexColor.a = 1.0;\n\
				}\n\
			}\n\
			else\n\
			{\n\
				if (texDrawOpaque)\n\
				{\n\
					if ( (polyMode != 1u) && (mainTexColor.a <= 0.999) )\n\
					{\n\
						discard;\n\
					}\n\
				}\n\
				else\n\
				{\n\
					if ( ((polyMode != 1u) && (mainTexColor.a * vtxColor.a > 0.999)) || ((polyMode == 1u) && (vtxColor.a > 0.999)) )\n\
					{\n\
						discard;\n\
					}\n\
				}\n\
			}\n\
			\n\
			newFragColor = mainTexColor * vtxColor;\n\
			\n\
			if (polyMode == 1u)\n\
			{\n\
				newFragColor.rgb = bool(polyEnableTexture) ? mix(vtxColor.rgb, mainTexColor.rgb, mainTexColor.a) : vtxColor.rgb;\n\
				newFragColor.a = vtxColor.a;\n\
			}\n\
			else if (polyMode == 2u)\n\
			{\n\
				vec3 newToonColor = state.toonColor[int((vtxColor.r * 31.0) + 0.5)].rgb;\n\
				newFragColor.rgb = (state.toonShadingMode == 0) ? mainTexColor.rgb * newToonColor.rgb : min((mainTexColor.rgb * vtxColor.r) + newToonColor.rgb, 1.0);\n\
			}\n\
			else if (polyMode == 3u)\n\
			{\n\
				newFragColor = vtxColor;\n\
			}\n\
			\n\
			if (newFragColor.a < 0.001 || (state.enableAlphaTest && newFragColor.a < state.alphaTestRef))\n\
			{\n\
				discard;\n\
			}\n\
			\n\
			newPolyID = vec4( float(polyID)/63.0, float(polyIsWireframe == 1u), 0.0, float(newFragColor.a > 0.999) );\n\
			newFogAttributes = vec4( float(polyEnableFog), 0.0, 0.0, float((newFragColor.a > 0.999) ? 1.0 : 0.5) );\n\
		}\n\
		\n\
		outFragColor = newFragColor;\n\
		outPolyID = newPolyID;\n\
		outFogAttributes = newFogAttributes;\n\
		gl_FragDepth = newFragDepthValue;\n\
	}\n\
"};

// Vertex shader for determining which pixels have a zero alpha, GLSL 1.50
static const char *GeometryZeroDstAlphaPixelMaskVtxShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 inPosition;\n\
	in vec2 inTexCoord0;\n\
	out vec2 texCoord;\n\
	\n\
	void main()\n\
	{\n\
		texCoord = inTexCoord0;\n\
		gl_Position = vec4(inPosition, 0.0, 1.0);\n\
	}\n\
"};

// Fragment shader for determining which pixels have a zero alpha, GLSL 1.50
static const char *GeometryZeroDstAlphaPixelMaskFragShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 texCoord;\n\
	uniform sampler2D texInFragColor;\n\
	\n\
	void main()\n\
	{\n\
		vec4 inFragColor = texture(texInFragColor, texCoord);\n\
		\n\
		if (inFragColor.a <= 0.001)\n\
		{\n\
			discard;\n\
		}\n\
	}\n\
"};

// Vertex shader for determining which pixels have a zero alpha, GLSL 1.50
static const char *MSGeometryZeroDstAlphaPixelMaskVtxShader_150 = {"\
	#version 150\n\
	#extension GL_ARB_sample_shading : require\n\
	\n\
	in vec2 inPosition;\n\
	in vec2 inTexCoord0;\n\
	uniform sampler2DMS texInFragColor;\n\
	out vec2 texCoord;\n\
	\n\
	void main()\n\
	{\n\
		texCoord = inTexCoord0 * textureSize(texInFragColor);\n\
		gl_Position = vec4(inPosition, 0.0, 1.0);\n\
	}\n\
"};

// Fragment shader for determining which pixels have a zero alpha, GLSL 1.50
static const char *MSGeometryZeroDstAlphaPixelMaskFragShader_150 = {"\
	#version 150\n\
	#extension GL_ARB_sample_shading : require\n\
	\n\
	in vec2 texCoord;\n\
	uniform sampler2DMS texInFragColor;\n\
	\n\
	void main()\n\
	{\n\
		vec4 inFragColor = texelFetch(texInFragColor, ivec2(texCoord), gl_SampleID);\n\
		\n\
		if (inFragColor.a <= 0.001)\n\
		{\n\
			discard;\n\
		}\n\
	}\n\
"};

// Vertex shader for applying edge marking, GLSL 1.50
static const char *EdgeMarkVtxShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 inPosition;\n\
	in vec2 inTexCoord0;\n\
	\n\
	layout (std140) uniform RenderStates\n\
	{\n\
		vec2 framebufferSize;\n\
		int toonShadingMode;\n\
		bool enableAlphaTest;\n\
		bool enableAntialiasing;\n\
		bool enableEdgeMarking;\n\
		bool enableFogAlphaOnly;\n\
		bool useWDepth;\n\
		int clearPolyID;\n\
		float clearDepth;\n\
		float alphaTestRef;\n\
		float fogOffset;\n\
		float fogStep;\n\
		float pad_0;\n\
		vec4 fogColor;\n\
		float fogDensity[32];\n\
		vec4 edgeColor[8];\n\
		vec4 toonColor[32];\n\
	} state;\n\
	\n\
	out vec2 texCoord[5];\n\
	\n\
	void main()\n\
	{\n\
		vec2 texInvScale = vec2(1.0/state.framebufferSize.x, 1.0/state.framebufferSize.y);\n\
		\n\
		texCoord[0] = inTexCoord0; // Center\n\
		texCoord[1] = inTexCoord0 + (vec2( 1.0, 0.0) * texInvScale); // Right\n\
		texCoord[2] = inTexCoord0 + (vec2( 0.0, 1.0) * texInvScale); // Down\n\
		texCoord[3] = inTexCoord0 + (vec2(-1.0, 0.0) * texInvScale); // Left\n\
		texCoord[4] = inTexCoord0 + (vec2( 0.0,-1.0) * texInvScale); // Up\n\
		\n\
		gl_Position = vec4(inPosition, 0.0, 1.0);\n\
	}\n\
"};

// Fragment shader for applying edge marking, GLSL 1.50
static const char *EdgeMarkFragShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 texCoord[5];\n\
	\n\
	layout (std140) uniform RenderStates\n\
	{\n\
		vec2 framebufferSize;\n\
		int toonShadingMode;\n\
		bool enableAlphaTest;\n\
		bool enableAntialiasing;\n\
		bool enableEdgeMarking;\n\
		bool enableFogAlphaOnly;\n\
		bool useWDepth;\n\
		int clearPolyID;\n\
		float clearDepth;\n\
		float alphaTestRef;\n\
		float fogOffset;\n\
		float fogStep;\n\
		float pad_0;\n\
		vec4 fogColor;\n\
		float fogDensity[32];\n\
		vec4 edgeColor[8];\n\
		vec4 toonColor[32];\n\
	} state;\n\
	\n\
	uniform sampler2D texInFragDepth;\n\
	uniform sampler2D texInPolyID;\n\
	\n\
	out vec4 outFragColor;\n\
	\n\
	void main()\n\
	{\n\
		vec4 polyIDInfo[5];\n\
		polyIDInfo[0] = texture(texInPolyID, texCoord[0]);\n\
		polyIDInfo[1] = texture(texInPolyID, texCoord[1]);\n\
		polyIDInfo[2] = texture(texInPolyID, texCoord[2]);\n\
		polyIDInfo[3] = texture(texInPolyID, texCoord[3]);\n\
		polyIDInfo[4] = texture(texInPolyID, texCoord[4]);\n\
		\n\
		bool isWireframe[5];\n\
		isWireframe[0] = bool(polyIDInfo[0].g);\n\
		\n\
		float depth[5];\n\
		depth[0] = texture(texInFragDepth, texCoord[0]).r;\n\
		depth[1] = texture(texInFragDepth, texCoord[1]).r;\n\
		depth[2] = texture(texInFragDepth, texCoord[2]).r;\n\
		depth[3] = texture(texInFragDepth, texCoord[3]).r;\n\
		depth[4] = texture(texInFragDepth, texCoord[4]).r;\n\
		\n\
		vec4 newEdgeColor = vec4(0.0, 0.0, 0.0, 0.0);\n\
		\n\
		if (!isWireframe[0])\n\
		{\n\
			int polyID[5];\n\
			polyID[0] = int((polyIDInfo[0].r * 63.0) + 0.5);\n\
			polyID[1] = int((polyIDInfo[1].r * 63.0) + 0.5);\n\
			polyID[2] = int((polyIDInfo[2].r * 63.0) + 0.5);\n\
			polyID[3] = int((polyIDInfo[3].r * 63.0) + 0.5);\n\
			polyID[4] = int((polyIDInfo[4].r * 63.0) + 0.5);\n\
			\n\
			isWireframe[1] = bool(polyIDInfo[1].g);\n\
			isWireframe[2] = bool(polyIDInfo[2].g);\n\
			isWireframe[3] = bool(polyIDInfo[3].g);\n\
			isWireframe[4] = bool(polyIDInfo[4].g);\n\
			\n\
			bool isEdgeMarkingClearValues = ((polyID[0] != state.clearPolyID) && (depth[0] < state.clearDepth) && !isWireframe[0]);\n\
			vec2 pixelCoord = texCoord[0] * state.framebufferSize;\n\
			\n\
			if ( ((pixelCoord.x >= state.framebufferSize.x-1.0) ? isEdgeMarkingClearValues : ((polyID[0] != polyID[1]) && (depth[0] >= depth[1]) && !isWireframe[1])) )\n\
			{\n\
				if (pixelCoord.x >= state.framebufferSize.x-1.0)\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[0]/8];\n\
				}\n\
				else\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[1]/8];\n\
				}\n\
			}\n\
			else if ( ((pixelCoord.y >= state.framebufferSize.y-1.0) ? isEdgeMarkingClearValues : ((polyID[0] != polyID[2]) && (depth[0] >= depth[2]) && !isWireframe[2])) )\n\
			{\n\
				if (pixelCoord.y >= state.framebufferSize.y-1.0)\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[0]/8];\n\
				}\n\
				else\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[2]/8];\n\
				}\n\
			}\n\
			else if ( ((pixelCoord.x < 1.0) ? isEdgeMarkingClearValues : ((polyID[0] != polyID[3]) && (depth[0] >= depth[3]) && !isWireframe[3])) )\n\
			{\n\
				if (pixelCoord.x < 1.0)\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[0]/8];\n\
				}\n\
				else\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[3]/8];\n\
				}\n\
			}\n\
			else if ( ((pixelCoord.y < 1.0) ? isEdgeMarkingClearValues : ((polyID[0] != polyID[4]) && (depth[0] >= depth[4]) && !isWireframe[4])) )\n\
			{\n\
				if (pixelCoord.y < 1.0)\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[0]/8];\n\
				}\n\
				else\n\
				{\n\
					newEdgeColor = state.edgeColor[polyID[4]/8];\n\
				}\n\
			}\n\
		}\n\
		\n\
		outFragColor = newEdgeColor;\n\
	}\n\
"};

// Vertex shader for applying fog, GLSL 1.50
static const char *FogVtxShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 inPosition;\n\
	in vec2 inTexCoord0;\n\
	out vec2 texCoord;\n\
	\n\
	void main()\n\
	{\n\
		texCoord = inTexCoord0;\n\
		gl_Position = vec4(inPosition, 0.0, 1.0);\n\
	}\n\
"};

// Fragment shader for applying fog, GLSL 1.50
static const char *FogFragShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 texCoord;\n\
	\n\
	layout (std140) uniform RenderStates\n\
	{\n\
		vec2 framebufferSize;\n\
		int toonShadingMode;\n\
		bool enableAlphaTest;\n\
		bool enableAntialiasing;\n\
		bool enableEdgeMarking;\n\
		bool enableFogAlphaOnly;\n\
		bool useWDepth;\n\
		int clearPolyID;\n\
		float clearDepth;\n\
		float alphaTestRef;\n\
		float fogOffset;\n\
		float fogStep;\n\
		float pad_0;\n\
		vec4 fogColor;\n\
		float fogDensity[32];\n\
		vec4 edgeColor[8];\n\
		vec4 toonColor[32];\n\
	} state;\n\
	\n\
	uniform sampler2D texInFragColor;\n\
	uniform sampler2D texInFragDepth;\n\
	uniform sampler2D texInFogAttributes;\n\
	\n\
	out vec4 outFragColor;\n\
	\n\
	void main()\n\
	{\n\
		vec4 inFragColor = texture(texInFragColor, texCoord);\n\
		vec4 inFogAttributes = texture(texInFogAttributes, texCoord);\n\
		bool polyEnableFog = (inFogAttributes.r > 0.999);\n\
		vec4 newFoggedColor = inFragColor;\n\
		\n\
		if (polyEnableFog)\n\
		{\n\
			float inFragDepth = texture(texInFragDepth, texCoord).r;\n\
			float fogMixWeight = 0.0;\n\
			\n\
			if (inFragDepth <= min(state.fogOffset + state.fogStep, 1.0))\n\
			{\n\
				fogMixWeight = state.fogDensity[0];\n\
			}\n\
			else if (inFragDepth >= min(state.fogOffset + (state.fogStep*32.0), 1.0))\n\
			{\n\
				fogMixWeight = state.fogDensity[31];\n\
			}\n\
			else\n\
			{\n\
				for (int i = 1; i < 32; i++)\n\
				{\n\
					float currentFogStep = min(state.fogOffset + (state.fogStep * float(i+1)), 1.0);\n\
					if (inFragDepth <= currentFogStep)\n\
					{\n\
						float previousFogStep = min(state.fogOffset + (state.fogStep * float(i)), 1.0);\n\
						fogMixWeight = mix(state.fogDensity[i-1], state.fogDensity[i], (inFragDepth - previousFogStep) / (currentFogStep - previousFogStep));\n\
						break;\n\
					}\n\
				}\n\
			}\n\
			\n\
			newFoggedColor = mix(inFragColor, (state.enableFogAlphaOnly) ? vec4(inFragColor.rgb, state.fogColor.a) : state.fogColor, fogMixWeight);\n\
		}\n\
		\n\
		outFragColor = newFoggedColor;\n\
	}\n\
"};

// Vertex shader for the final framebuffer, GLSL 1.50
static const char *FramebufferOutputVtxShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 inPosition;\n\
	in vec2 inTexCoord0;\n\
	\n\
	uniform sampler2D texInFragColor;\n\
	\n\
	out vec2 texCoord;\n\
	\n\
	void main()\n\
	{\n\
		float framebufferHeight = float( textureSize(texInFragColor, 0).y );\n\
		texCoord = vec2(inTexCoord0.x, (framebufferHeight - (framebufferHeight * inTexCoord0.y)) / framebufferHeight);\n\
		gl_Position = vec4(inPosition, 0.0, 1.0);\n\
	}\n\
"};

// Fragment shader for the final framebuffer, GLSL 1.50
static const char *FramebufferOutputFragShader_150 = {"\
	#version 150\n\
	\n\
	in vec2 texCoord;\n\
	\n\
	uniform sampler2D texInFragColor;\n\
	\n\
	out vec4 outFragColor;\n\
	\n\
	void main()\n\
	{\n\
		// Note that we swap B and R since pixel readbacks are done in BGRA format for fastest\n\
		// performance. The final color is still in RGBA format.\n\
		vec4 colorRGBA6665 = texture(texInFragColor, texCoord).bgra;\n\
		colorRGBA6665     = floor((colorRGBA6665 * 255.0) + 0.5);\n\
		colorRGBA6665.rgb = floor(colorRGBA6665.rgb / 4.0);\n\
		colorRGBA6665.a   = floor(colorRGBA6665.a   / 8.0);\n\
		\n\
		outFragColor = (colorRGBA6665 / 255.0);\n\
	}\n\
"};

void OGLCreateRenderer_3_2(OpenGLRenderer **rendererPtr)
{
	if (IsVersionSupported(3, 2, 0))
	{
		*rendererPtr = new OpenGLRenderer_3_2;
		(*rendererPtr)->SetVersion(3, 2, 0);
	}
}

OpenGLRenderer_3_2::~OpenGLRenderer_3_2()
{
	glFinish();
	
	DestroyVAOs();
	DestroyFBOs();
	DestroyMultisampledFBO();
}

Render3DError OpenGLRenderer_3_2::InitExtensions()
{
	OGLRenderRef &OGLRef = *this->ref;
	Render3DError error = OGLERROR_NOERR;
	
	// Get OpenGL extensions
	std::set<std::string> oglExtensionSet;
	this->GetExtensionSet(&oglExtensionSet);
	
	// Get host GPU device properties
	GLfloat maxAnisotropyOGL = 1.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropyOGL);
	this->_deviceInfo.maxAnisotropy = (float)maxAnisotropyOGL;
	
	this->_deviceInfo.isEdgeMarkSupported = true;
	this->_deviceInfo.isFogSupported = true;
	
	// Initialize OpenGL
	this->InitTables();
	
	glGenTextures(1, &OGLRef.texFinalColorID);
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_FinalColor);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texFinalColorID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	glActiveTexture(GL_TEXTURE0);
	
	// Load and create shaders. Return on any error, since v3.2 Core Profile makes shaders mandatory.
	this->isShaderSupported	= true;
	
	// OpenGL v3.2 Core Profile should have all the necessary features to be able to flip and convert the framebuffer.
	this->willFlipOnlyFramebufferOnGPU = true;
	this->willFlipAndConvertFramebufferOnGPU = true;
	
	this->isSampleShadingSupported = this->IsExtensionPresent(&oglExtensionSet, "GL_ARB_sample_shading");
	
	error = this->InitGeometryProgram(GeometryVtxShader_150, GeometryFragShader_150,
									  GeometryZeroDstAlphaPixelMaskVtxShader_150, GeometryZeroDstAlphaPixelMaskFragShader_150,
									  MSGeometryZeroDstAlphaPixelMaskVtxShader_150, MSGeometryZeroDstAlphaPixelMaskFragShader_150);
	if (error != OGLERROR_NOERR)
	{
		this->DestroyGeometryProgram();
		this->isShaderSupported = false;
		
		return error;
	}
	
	this->willUsePerSampleZeroDstPass = this->isSampleShadingSupported && (OGLRef.programMSGeometryZeroDstAlphaID != 0);
	
	error = this->InitPostprocessingPrograms(EdgeMarkVtxShader_150,
											 EdgeMarkFragShader_150,
											 FogVtxShader_150,
											 FogFragShader_150,
											 FramebufferOutputVtxShader_150,
											 FramebufferOutputFragShader_150,
											 FramebufferOutputFragShader_150);
	if (error != OGLERROR_NOERR)
	{
		this->DestroyPostprocessingPrograms();
		this->DestroyGeometryProgram();
		this->isShaderSupported = false;
		
		return error;
	}
	
	this->isVBOSupported = true;
	this->CreateVBOs();
	
	this->isPBOSupported = true;
	this->CreatePBOs();
	
	this->isVAOSupported = true;
	this->CreateVAOs();
	
	// Load and create FBOs. Return on any error, since v3.2 Core Profile makes FBOs mandatory.
	this->isFBOSupported = true;
	error = this->CreateFBOs();
	if (error != OGLERROR_NOERR)
	{
		this->isFBOSupported = false;
		return error;
	}
	
	this->isMultisampledFBOSupported = true;
	this->_selectedMultisampleSize = CommonSettings.GFX3D_Renderer_MultisampleSize;
	
	GLint maxSamplesOGL = 0;
	glGetIntegerv(GL_MAX_SAMPLES, &maxSamplesOGL);
	this->_deviceInfo.maxSamples = (u8)maxSamplesOGL;
	
	if (this->_deviceInfo.maxSamples >= 2)
	{
		// Try and initialize the multisampled FBOs with the GFX3D_Renderer_MultisampleSize.
		// However, if the client has this set to 0, then set sampleSize to 2 in order to
		// force the generation and the attachments of the buffers at a meaningful sample
		// size. If GFX3D_Renderer_MultisampleSize is 0, then we can deallocate the buffer
		// memory afterwards.
		GLsizei sampleSize = this->GetLimitedMultisampleSize();
		if (sampleSize == 0)
		{
			sampleSize = 2;
		}
		
		error = this->CreateMultisampledFBO(sampleSize);
		if (error != OGLERROR_NOERR)
		{
			this->isMultisampledFBOSupported = false;
		}
		
		// If GFX3D_Renderer_MultisampleSize is 0, then we can deallocate the buffers now
		// in order to save some memory.
		if (this->_selectedMultisampleSize == 0)
		{
			this->ResizeMultisampledFBOs(0);
		}
	}
	else
	{
		this->isMultisampledFBOSupported = false;
		INFO("OpenGL: Driver does not support at least 2x multisampled FBOs.\n");
	}
	
	this->_enableMultisampledRendering = ((this->_selectedMultisampleSize >= 2) && this->isMultisampledFBOSupported);
	
	this->InitFinalRenderStates(&oglExtensionSet); // This must be done last
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitEdgeMarkProgramBindings()
{
	OGLRenderRef &OGLRef = *this->ref;
	glBindAttribLocation(OGLRef.programEdgeMarkID, OGLVertexAttributeID_Position, "inPosition");
	glBindAttribLocation(OGLRef.programEdgeMarkID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	glBindFragDataLocation(OGLRef.programEdgeMarkID, 0, "outFragColor");
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitEdgeMarkProgramShaderLocations()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glUseProgram(OGLRef.programEdgeMarkID);
	
	const GLuint uniformBlockRenderStates = glGetUniformBlockIndex(OGLRef.programEdgeMarkID, "RenderStates");
	glUniformBlockBinding(OGLRef.programEdgeMarkID, uniformBlockRenderStates, OGLBindingPointID_RenderStates);
	
	const GLint uniformTexGDepth				= glGetUniformLocation(OGLRef.programEdgeMarkID, "texInFragDepth");
	const GLint uniformTexGPolyID				= glGetUniformLocation(OGLRef.programEdgeMarkID, "texInPolyID");
	glUniform1i(uniformTexGDepth, OGLTextureUnitID_DepthStencil);
	glUniform1i(uniformTexGPolyID, OGLTextureUnitID_GPolyID);
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitFogProgramBindings()
{
	OGLRenderRef &OGLRef = *this->ref;
	glBindAttribLocation(OGLRef.programFogID, OGLVertexAttributeID_Position, "inPosition");
	glBindAttribLocation(OGLRef.programFogID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	glBindFragDataLocation(OGLRef.programFogID, 0, "outFragColor");
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitFogProgramShaderLocations()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glUseProgram(OGLRef.programFogID);
	
	const GLuint uniformBlockRenderStates = glGetUniformBlockIndex(OGLRef.programFogID, "RenderStates");
	glUniformBlockBinding(OGLRef.programFogID, uniformBlockRenderStates, OGLBindingPointID_RenderStates);
	
	const GLint uniformTexGColor	= glGetUniformLocation(OGLRef.programFogID, "texInFragColor");
	const GLint uniformTexGDepth	= glGetUniformLocation(OGLRef.programFogID, "texInFragDepth");
	const GLint uniformTexGFog		= glGetUniformLocation(OGLRef.programFogID, "texInFogAttributes");
	glUniform1i(uniformTexGColor, OGLTextureUnitID_GColor);
	glUniform1i(uniformTexGDepth, OGLTextureUnitID_DepthStencil);
	glUniform1i(uniformTexGFog, OGLTextureUnitID_FogAttr);
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitFramebufferOutputProgramBindings()
{
	OGLRenderRef &OGLRef = *this->ref;
	glBindAttribLocation(OGLRef.programFramebufferRGBA6665OutputID, OGLVertexAttributeID_Position, "inPosition");
	glBindAttribLocation(OGLRef.programFramebufferRGBA6665OutputID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	glBindAttribLocation(OGLRef.programFramebufferRGBA8888OutputID, OGLVertexAttributeID_Position, "inPosition");
	glBindAttribLocation(OGLRef.programFramebufferRGBA8888OutputID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	glBindFragDataLocation(OGLRef.programFramebufferRGBA6665OutputID, 0, "outFragColor");
	glBindFragDataLocation(OGLRef.programFramebufferRGBA8888OutputID, 0, "outFragColor");
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitFramebufferOutputShaderLocations()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glUseProgram(OGLRef.programFramebufferRGBA6665OutputID);
	OGLRef.uniformTexInFragColor_ConvertRGBA6665 = glGetUniformLocation(OGLRef.programFramebufferRGBA6665OutputID, "texInFragColor");
	glUniform1i(OGLRef.uniformTexInFragColor_ConvertRGBA6665, OGLTextureUnitID_FinalColor);
	
	glUseProgram(OGLRef.programFramebufferRGBA8888OutputID);
	OGLRef.uniformTexInFragColor_ConvertRGBA8888 = glGetUniformLocation(OGLRef.programFramebufferRGBA8888OutputID, "texInFragColor");
	glUniform1i(OGLRef.uniformTexInFragColor_ConvertRGBA8888, OGLTextureUnitID_FinalColor);
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::CreateFBOs()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	// Set up FBO render targets
	glGenTextures(1, &OGLRef.texCIColorID);
	glGenTextures(1, &OGLRef.texCIFogAttrID);
	glGenTextures(1, &OGLRef.texCIPolyID);
	glGenTextures(1, &OGLRef.texCIDepthStencilID);
	
	glGenTextures(1, &OGLRef.texGColorID);
	glGenTextures(1, &OGLRef.texGFogAttrID);
	glGenTextures(1, &OGLRef.texGPolyID);
	glGenTextures(1, &OGLRef.texGDepthStencilID);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_DepthStencil);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texGDepthStencilID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, this->_framebufferWidth, this->_framebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_GColor);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texGColorID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_GPolyID);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texGPolyID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_FogAttr);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texGFogAttrID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0);
	
	memset(OGLRef.workingCIColorBuffer, 0, sizeof(OGLRef.workingCIColorBuffer));
	glBindTexture(GL_TEXTURE_2D, OGLRef.texCIColorID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GPU_FRAMEBUFFER_NATIVE_WIDTH, GPU_FRAMEBUFFER_NATIVE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, OGLRef.workingCIColorBuffer);
	
	glBindTexture(GL_TEXTURE_2D, OGLRef.texCIDepthStencilID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, GPU_FRAMEBUFFER_NATIVE_WIDTH, GPU_FRAMEBUFFER_NATIVE_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glBindTexture(GL_TEXTURE_2D, OGLRef.texCIPolyID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GPU_FRAMEBUFFER_NATIVE_WIDTH, GPU_FRAMEBUFFER_NATIVE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glBindTexture(GL_TEXTURE_2D, OGLRef.texCIFogAttrID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GPU_FRAMEBUFFER_NATIVE_WIDTH, GPU_FRAMEBUFFER_NATIVE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	// Set up FBOs
	glGenFramebuffers(1, &OGLRef.fboClearImageID);
	glGenFramebuffers(1, &OGLRef.fboRenderID);
	glGenFramebuffers(1, &OGLRef.fboPostprocessID);
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboClearImageID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, OGLRef.texCIColorID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, OGLRef.texCIPolyID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, OGLRef.texCIFogAttrID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, OGLRef.texCIDepthStencilID, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("OpenGL: Failed to create FBOs!\n");
		this->DestroyFBOs();
		
		return OGLERROR_FBO_CREATE_ERROR;
	}
	
	glDrawBuffers(3, RenderDrawList);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboRenderID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, OGLRef.texGColorID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, OGLRef.texGPolyID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, OGLRef.texGFogAttrID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, OGLRef.texGDepthStencilID, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("OpenGL: Failed to create FBOs!\n");
		this->DestroyFBOs();
		
		return OGLERROR_FBO_CREATE_ERROR;
	}
	
	glDrawBuffers(3, RenderDrawList);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboPostprocessID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, OGLRef.texGColorID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, OGLRef.texFinalColorID, 0);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("OpenGL: Failed to create FBOs!\n");
		this->DestroyFBOs();
		
		return OGLERROR_FBO_CREATE_ERROR;
	}
	
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	
	OGLRef.selectedRenderingFBO = OGLRef.fboRenderID;
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.selectedRenderingFBO);
	INFO("OpenGL: Successfully created FBOs.\n");
	
	return OGLERROR_NOERR;
}

void OpenGLRenderer_3_2::DestroyFBOs()
{
	if (!this->isFBOSupported)
	{
		return;
	}
	
	OGLRenderRef &OGLRef = *this->ref;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &OGLRef.fboClearImageID);
	glDeleteFramebuffers(1, &OGLRef.fboRenderID);
	glDeleteFramebuffers(1, &OGLRef.fboPostprocessID);
	glDeleteTextures(1, &OGLRef.texCIColorID);
	glDeleteTextures(1, &OGLRef.texCIFogAttrID);
	glDeleteTextures(1, &OGLRef.texCIPolyID);
	glDeleteTextures(1, &OGLRef.texCIDepthStencilID);
	glDeleteTextures(1, &OGLRef.texGColorID);
	glDeleteTextures(1, &OGLRef.texGPolyID);
	glDeleteTextures(1, &OGLRef.texGFogAttrID);
	glDeleteTextures(1, &OGLRef.texGDepthStencilID);
	
	OGLRef.fboClearImageID = 0;
	OGLRef.fboRenderID = 0;
	OGLRef.fboPostprocessID = 0;
	
	this->isFBOSupported = false;
}

Render3DError OpenGLRenderer_3_2::CreateMultisampledFBO(GLsizei numSamples)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	// Set up FBO render targets
	glGenRenderbuffers(1, &OGLRef.rboMSGPolyID);
	glGenRenderbuffers(1, &OGLRef.rboMSGFogAttrID);
	glGenRenderbuffers(1, &OGLRef.rboMSGDepthStencilID);
	
	if (this->willUsePerSampleZeroDstPass)
	{
		glGenTextures(1, &OGLRef.texMSGColorID);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, OGLRef.texMSGColorID);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight, GL_TRUE);
	}
	else
	{
		glGenRenderbuffers(1, &OGLRef.rboMSGColorID);
		glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGColorID);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight);
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGPolyID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGFogAttrID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA, this->_framebufferWidth, this->_framebufferHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGDepthStencilID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_DEPTH24_STENCIL8, this->_framebufferWidth, this->_framebufferHeight);
	
	// Set up multisampled rendering FBO
	glGenFramebuffers(1, &OGLRef.fboMSIntermediateRenderID);
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboMSIntermediateRenderID);
	
	if (this->willUsePerSampleZeroDstPass)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, OGLRef.texMSGColorID, 0);
	}
	else
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, OGLRef.rboMSGColorID);
	}
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, OGLRef.rboMSGPolyID);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, OGLRef.rboMSGFogAttrID);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, OGLRef.rboMSGDepthStencilID);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		INFO("OpenGL: Failed to create multisampled FBO. Multisample antialiasing will be disabled.\n");
		this->DestroyMultisampledFBO();
		
		return OGLERROR_FBO_CREATE_ERROR;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboRenderID);
	INFO("OpenGL: Successfully created multisampled FBO.\n");
	
	return OGLERROR_NOERR;
}

void OpenGLRenderer_3_2::DestroyMultisampledFBO()
{
	if (!this->isMultisampledFBOSupported)
	{
		return;
	}
	
	OGLRenderRef &OGLRef = *this->ref;
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &OGLRef.fboMSIntermediateRenderID);
	glDeleteTextures(1, &OGLRef.texMSGColorID);
	glDeleteRenderbuffers(1, &OGLRef.rboMSGColorID);
	glDeleteRenderbuffers(1, &OGLRef.rboMSGPolyID);
	glDeleteRenderbuffers(1, &OGLRef.rboMSGFogAttrID);
	glDeleteRenderbuffers(1, &OGLRef.rboMSGDepthStencilID);
	
	OGLRef.fboMSIntermediateRenderID = 0;
	
	this->isMultisampledFBOSupported = false;
}

void OpenGLRenderer_3_2::ResizeMultisampledFBOs(GLsizei numSamples)
{
	OGLRenderRef &OGLRef = *this->ref;
	GLsizei w = this->_framebufferWidth;
	GLsizei h = this->_framebufferHeight;
	
	if ( !this->isMultisampledFBOSupported ||
		(numSamples == 1) ||
		(w < GPU_FRAMEBUFFER_NATIVE_WIDTH) || (h < GPU_FRAMEBUFFER_NATIVE_HEIGHT) )
	{
		return;
	}
	
	if (numSamples == 0)
	{
		w = 0;
		h = 0;
		numSamples = 2;
	}
	
	if (this->willUsePerSampleZeroDstPass)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, OGLRef.texMSGColorID);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, GL_RGBA, w, h, GL_TRUE);
	}
	else
	{
		glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGColorID);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA, w, h);
	}
	
	glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGPolyID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGFogAttrID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_RGBA, w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, OGLRef.rboMSGDepthStencilID);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, numSamples, GL_DEPTH24_STENCIL8, w, h);
}

Render3DError OpenGLRenderer_3_2::CreateVAOs()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glGenVertexArrays(1, &OGLRef.vaoGeometryStatesID);
	glGenVertexArrays(1, &OGLRef.vaoPostprocessStatesID);
	
	glBindVertexArray(OGLRef.vaoGeometryStatesID);
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboGeometryVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboGeometryIndexID);
	
	glEnableVertexAttribArray(OGLVertexAttributeID_Position);
	glEnableVertexAttribArray(OGLVertexAttributeID_TexCoord0);
	glEnableVertexAttribArray(OGLVertexAttributeID_Color);
	glVertexAttribPointer(OGLVertexAttributeID_Position, 4, GL_FLOAT, GL_FALSE, sizeof(VERT), (const GLvoid *)offsetof(VERT, coord));
	glVertexAttribPointer(OGLVertexAttributeID_TexCoord0, 2, GL_FLOAT, GL_FALSE, sizeof(VERT), (const GLvoid *)offsetof(VERT, texcoord));
	glVertexAttribPointer(OGLVertexAttributeID_Color, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VERT), (const GLvoid *)offsetof(VERT, color));
	
	glBindVertexArray(0);
	
	glBindVertexArray(OGLRef.vaoPostprocessStatesID);
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboPostprocessVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboPostprocessIndexID);
	
	glEnableVertexAttribArray(OGLVertexAttributeID_Position);
	glEnableVertexAttribArray(OGLVertexAttributeID_TexCoord0);
	glVertexAttribPointer(OGLVertexAttributeID_Position, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(OGLVertexAttributeID_TexCoord0, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(GLfloat) * 8));
	
	glBindVertexArray(0);
	
	return OGLERROR_NOERR;
}

void OpenGLRenderer_3_2::DestroyVAOs()
{
	if (!this->isVAOSupported)
	{
		return;
	}
	
	OGLRenderRef &OGLRef = *this->ref;
	
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &OGLRef.vaoGeometryStatesID);
	glDeleteVertexArrays(1, &OGLRef.vaoPostprocessStatesID);
	
	this->isVAOSupported = false;
}

Render3DError OpenGLRenderer_3_2::InitGeometryProgramBindings()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glBindAttribLocation(OGLRef.programGeometryID, OGLVertexAttributeID_Position, "inPosition");
	glBindAttribLocation(OGLRef.programGeometryID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	glBindAttribLocation(OGLRef.programGeometryID, OGLVertexAttributeID_Color, "inColor");
	glBindFragDataLocation(OGLRef.programGeometryID, 0, "outFragColor");
	glBindFragDataLocation(OGLRef.programGeometryID, 1, "outPolyID");
	glBindFragDataLocation(OGLRef.programGeometryID, 2, "outFogAttributes");
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitGeometryProgramShaderLocations()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glUseProgram(OGLRef.programGeometryID);
	
	// Set up render states UBO
	const GLuint uniformBlockRenderStates	= glGetUniformBlockIndex(OGLRef.programGeometryID, "RenderStates");
	glUniformBlockBinding(OGLRef.programGeometryID, uniformBlockRenderStates, OGLBindingPointID_RenderStates);
	
	GLint uboSize = 0;
	glGetActiveUniformBlockiv(OGLRef.programGeometryID, uniformBlockRenderStates, GL_UNIFORM_BLOCK_DATA_SIZE, &uboSize);
	assert(uboSize == sizeof(OGLRenderStates));
	
	glGenBuffers(1, &OGLRef.uboRenderStatesID);
	glBindBuffer(GL_UNIFORM_BUFFER, OGLRef.uboRenderStatesID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(OGLRenderStates), NULL, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, OGLBindingPointID_RenderStates, OGLRef.uboRenderStatesID);
	
	// Set up poly states TBO
	glGenBuffers(1, &OGLRef.tboPolyStatesID);
	glBindBuffer(GL_TEXTURE_BUFFER, OGLRef.tboPolyStatesID);
	glBufferData(GL_TEXTURE_BUFFER, POLYLIST_SIZE * sizeof(OGLPolyStates), NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	
	glGenTextures(1, &OGLRef.texPolyStatesID);
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_PolyStates);
	glBindTexture(GL_TEXTURE_BUFFER, OGLRef.texPolyStatesID);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8UI, OGLRef.tboPolyStatesID);
	glActiveTexture(GL_TEXTURE0);
	
	const GLint uniformTexRenderObject		= glGetUniformLocation(OGLRef.programGeometryID, "texRenderObject");
	const GLint uniformTexBufferPolyStates	= glGetUniformLocation(OGLRef.programGeometryID, "PolyStates");
	glUniform1i(uniformTexRenderObject, 0);
	glUniform1i(uniformTexBufferPolyStates, OGLTextureUnitID_PolyStates);
	
	OGLRef.uniformTexDrawOpaque				= glGetUniformLocation(OGLRef.programGeometryID, "texDrawOpaque");
	OGLRef.uniformPolyDrawShadow			= glGetUniformLocation(OGLRef.programGeometryID, "polyDrawShadow");
	OGLRef.uniformPolyStateIndex			= glGetUniformLocation(OGLRef.programGeometryID, "polyIndex");
	OGLRef.uniformPolyDepthOffsetMode		= glGetUniformLocation(OGLRef.programGeometryID, "polyDepthOffsetMode");
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitGeometryZeroDstAlphaProgramBindings()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glBindAttribLocation(OGLRef.programGeometryZeroDstAlphaID, OGLVertexAttributeID_Position, "inPosition");
	glBindAttribLocation(OGLRef.programGeometryZeroDstAlphaID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	
	if (OGLRef.programMSGeometryZeroDstAlphaID != 0)
	{
		glBindAttribLocation(OGLRef.programMSGeometryZeroDstAlphaID, OGLVertexAttributeID_Position, "inPosition");
		glBindAttribLocation(OGLRef.programMSGeometryZeroDstAlphaID, OGLVertexAttributeID_TexCoord0, "inTexCoord0");
	}
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::InitGeometryZeroDstAlphaProgramShaderLocations()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glUseProgram(OGLRef.programGeometryZeroDstAlphaID);
	GLint uniformTexGColor = glGetUniformLocation(OGLRef.programGeometryZeroDstAlphaID, "texInFragColor");
	glUniform1i(uniformTexGColor, OGLTextureUnitID_GColor);
	
	if (OGLRef.programMSGeometryZeroDstAlphaID != 0)
	{
		glUseProgram(OGLRef.programMSGeometryZeroDstAlphaID);
		uniformTexGColor = glGetUniformLocation(OGLRef.programMSGeometryZeroDstAlphaID, "texInFragColor");
		glUniform1i(uniformTexGColor, 0);
	}
	
	return OGLERROR_NOERR;
}

void OpenGLRenderer_3_2::DestroyGeometryProgram()
{
	if (!this->isShaderSupported)
	{
		return;
	}
	
	OGLRenderRef &OGLRef = *this->ref;
	
	glUseProgram(0);
	
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glDeleteBuffers(1, &OGLRef.uboRenderStatesID);
	glDeleteBuffers(1, &OGLRef.tboPolyStatesID);
	
	glDetachShader(OGLRef.programGeometryID, OGLRef.vertexGeometryShaderID);
	glDetachShader(OGLRef.programGeometryID, OGLRef.fragmentGeometryShaderID);
	glDetachShader(OGLRef.programGeometryZeroDstAlphaID, OGLRef.vtxShaderGeometryZeroDstAlphaID);
	glDetachShader(OGLRef.programGeometryZeroDstAlphaID, OGLRef.fragShaderGeometryZeroDstAlphaID);
	glDetachShader(OGLRef.programMSGeometryZeroDstAlphaID, OGLRef.vtxShaderMSGeometryZeroDstAlphaID);
	glDetachShader(OGLRef.programMSGeometryZeroDstAlphaID, OGLRef.fragShaderMSGeometryZeroDstAlphaID);
	
	glDeleteProgram(OGLRef.programGeometryID);
	glDeleteProgram(OGLRef.programGeometryZeroDstAlphaID);
	glDeleteProgram(OGLRef.programMSGeometryZeroDstAlphaID);
	
	glDeleteShader(OGLRef.vertexGeometryShaderID);
	glDeleteShader(OGLRef.fragmentGeometryShaderID);
	glDeleteShader(OGLRef.vtxShaderGeometryZeroDstAlphaID);
	glDeleteShader(OGLRef.fragShaderGeometryZeroDstAlphaID);
	glDeleteShader(OGLRef.vtxShaderMSGeometryZeroDstAlphaID);
	glDeleteShader(OGLRef.fragShaderMSGeometryZeroDstAlphaID);
	
	OGLRef.uboRenderStatesID = 0;
	OGLRef.tboPolyStatesID = 0;
	
	OGLRef.programGeometryID = 0;
	OGLRef.programGeometryZeroDstAlphaID = 0;
	OGLRef.programMSGeometryZeroDstAlphaID = 0;
	
	OGLRef.vertexGeometryShaderID = 0;
	OGLRef.fragmentGeometryShaderID = 0;
	OGLRef.vtxShaderGeometryZeroDstAlphaID = 0;
	OGLRef.fragShaderGeometryZeroDstAlphaID = 0;
	OGLRef.vtxShaderMSGeometryZeroDstAlphaID = 0;
	OGLRef.fragShaderMSGeometryZeroDstAlphaID = 0;
	
	this->DestroyToonTable();
}

void OpenGLRenderer_3_2::GetExtensionSet(std::set<std::string> *oglExtensionSet)
{
	GLint extensionCount = 0;
	
	glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
	for (size_t i = 0; i < extensionCount; i++)
	{
		std::string extensionName = std::string((const char *)glGetStringi(GL_EXTENSIONS, i));
		oglExtensionSet->insert(extensionName);
	}
}

Render3DError OpenGLRenderer_3_2::EnableVertexAttributes()
{
	glBindVertexArray(this->ref->vaoGeometryStatesID);
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::DisableVertexAttributes()
{
	glBindVertexArray(0);
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::ZeroDstAlphaPass(const POLYLIST *polyList, const INDEXLIST *indexList, bool enableAlphaBlending, size_t indexOffset, POLYGON_ATTR lastPolyAttr)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	// Pre Pass: Fill in the stencil buffer based on the alpha of the current framebuffer color.
	// Fully transparent pixels (alpha == 0) -- Set stencil buffer to 0
	// All other pixels (alpha != 0) -- Set stencil buffer to 1
	
	this->DisableVertexAttributes();
	
	glDepthMask(GL_FALSE);
	glStencilMask(0x40);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	
	const bool isRunningMSAA = this->isMultisampledFBOSupported && (OGLRef.selectedRenderingFBO == OGLRef.fboMSIntermediateRenderID);
	const bool isRunningMSAAWithPerSampleShading = isRunningMSAA && this->willUsePerSampleZeroDstPass; // Doing per-sample shading should be a little more accurate than not doing so.
	
	if (isRunningMSAA && !isRunningMSAAWithPerSampleShading)
	{
		// Just downsample the color buffer now so that we have some texture data to sample from in the non-multisample shader.
		// This is not perfectly pixel accurate, but it's better than nothing.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OGLRef.fboRenderID);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.selectedRenderingFBO);
	}
	
	glUseProgram((isRunningMSAAWithPerSampleShading) ? OGLRef.programMSGeometryZeroDstAlphaID : OGLRef.programGeometryZeroDstAlphaID);
	glViewport(0, 0, this->_framebufferWidth, this->_framebufferHeight);
	glDisable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	glStencilFunc(GL_ALWAYS, 0x40, 0x40);
	glDrawBuffer(GL_NONE);
	
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboPostprocessVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboPostprocessIndexID);
	glBindVertexArray(OGLRef.vaoPostprocessStatesID);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	glBindVertexArray(0);
	
	// Setup for multiple pass alpha poly drawing
	glUseProgram(OGLRef.programGeometryID);
	glUniform1i(OGLRef.uniformTexDrawOpaque, GL_FALSE);
	glUniform1i(OGLRef.uniformPolyDrawShadow, GL_FALSE);
	
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboGeometryVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboGeometryIndexID);
	this->EnableVertexAttributes();
	
	// Draw the alpha polys, touching fully transparent pixels only once.
	static const GLenum RenderAlphaDrawList[3] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE};
	glDrawBuffers(3, RenderAlphaDrawList);
	glEnable(GL_DEPTH_TEST);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
	glStencilFunc(GL_NOTEQUAL, 0x40, 0x40);
	
	this->DrawPolygonsForIndexRange<OGLPolyDrawMode_ZeroAlphaPass>(polyList, indexList, polyList->opaqueCount, polyList->count - 1, indexOffset, lastPolyAttr);
	
	// Restore OpenGL states back to normal.
	glDrawBuffers(3, RenderDrawList);
	glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0xFF);
	
	if (enableAlphaBlending)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::DownsampleFBO()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	if (this->isMultisampledFBOSupported && (OGLRef.selectedRenderingFBO == OGLRef.fboMSIntermediateRenderID))
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, OGLRef.fboMSIntermediateRenderID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OGLRef.fboRenderID);
		
		// Blit the color and depth buffers
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		
		// Blit the polygon ID buffer
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		// Blit the fog buffer
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		// Reset framebuffer targets
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffers(3, RenderDrawList);
		glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboRenderID);
	}
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::ReadBackPixels()
{
	OGLRenderRef &OGLRef = *this->ref;
	
	if (this->_outputFormat == NDSColorFormat_BGR666_Rev)
	{
		// Both flips and converts the framebuffer on the GPU. No additional postprocessing
		// should be necessary at this point.
		glUseProgram(OGLRef.programFramebufferRGBA6665OutputID);
		glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboPostprocessID);
		
		if (this->_lastTextureDrawTarget == OGLTextureUnitID_GColor)
		{
			glUniform1i(OGLRef.uniformTexInFragColor_ConvertRGBA6665, OGLTextureUnitID_GColor);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			this->_lastTextureDrawTarget = OGLTextureUnitID_FinalColor;
		}
		else
		{
			glUniform1i(OGLRef.uniformTexInFragColor_ConvertRGBA6665, OGLTextureUnitID_FinalColor);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			this->_lastTextureDrawTarget = OGLTextureUnitID_GColor;
		}
		
		glViewport(0, 0, this->_framebufferWidth, this->_framebufferHeight);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		
		glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboPostprocessVtxID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboPostprocessIndexID);
		glBindVertexArray(OGLRef.vaoPostprocessStatesID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
		glBindVertexArray(0);
		
		if (this->_mappedFramebuffer != NULL)
		{
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			this->_mappedFramebuffer = NULL;
		}
		
		glReadPixels(0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
	}
	else
	{
		// Just flips the framebuffer in Y to match the coordinates of OpenGL and the NDS hardware.
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OGLRef.fboPostprocessID);
		
		if (this->_lastTextureDrawTarget == OGLTextureUnitID_GColor)
		{
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, this->_framebufferHeight, this->_framebufferWidth, 0, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, OGLRef.fboPostprocessID);
			glReadBuffer(GL_COLOR_ATTACHMENT1);
		}
		else
		{
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, this->_framebufferHeight, this->_framebufferWidth, 0, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, OGLRef.fboPostprocessID);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
		}
		
		// Read back the pixels in RGBA format, since an OpenGL 3.2 device should be able to read back this
		// format without a performance penalty.
		if (this->_mappedFramebuffer != NULL)
		{
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
			this->_mappedFramebuffer = NULL;
		}
		
		glReadPixels(0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}
	
	this->_pixelReadNeedsFinish = true;
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::BeginRender(const GFX3D &engine)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	if (!BEGINGL())
	{
		return OGLERROR_BEGINGL_FAILED;
	}
	
	// Since glReadPixels() is called at the end of every render, we know that rendering
	// must be synchronized at that time. Therefore, GL_MAP_UNSYNCHRONIZED_BIT should be
	// safe to use.
	
	glBindBuffer(GL_UNIFORM_BUFFER, OGLRef.uboRenderStatesID);
	OGLRenderStates *state = (OGLRenderStates *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(OGLRenderStates), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	
	state->framebufferSize.x = this->_framebufferWidth;
	state->framebufferSize.y = this->_framebufferHeight;
	state->toonShadingMode = engine.renderState.shading;
	state->enableAlphaTest = (engine.renderState.enableAlphaTest) ? GL_TRUE : GL_FALSE;
	state->enableAntialiasing = (engine.renderState.enableAntialiasing) ? GL_TRUE : GL_FALSE;
	state->enableEdgeMarking = (this->_enableEdgeMark) ? GL_TRUE : GL_FALSE;
	state->enableFogAlphaOnly = (engine.renderState.enableFogAlphaOnly) ? GL_TRUE : GL_FALSE;
	state->useWDepth = (engine.renderState.wbuffer) ? GL_TRUE : GL_FALSE;
	state->clearPolyID = this->_clearAttributes.opaquePolyID;
	state->clearDepth = (GLfloat)this->_clearAttributes.depth / (GLfloat)0x00FFFFFF;
	state->alphaTestRef = divide5bitBy31_LUT[engine.renderState.alphaTestRef];
	state->fogColor.r = divide5bitBy31_LUT[(engine.renderState.fogColor      ) & 0x0000001F];
	state->fogColor.g = divide5bitBy31_LUT[(engine.renderState.fogColor >>  5) & 0x0000001F];
	state->fogColor.b = divide5bitBy31_LUT[(engine.renderState.fogColor >> 10) & 0x0000001F];
	state->fogColor.a = divide5bitBy31_LUT[(engine.renderState.fogColor >> 16) & 0x0000001F];
	state->fogOffset = (GLfloat)(engine.renderState.fogOffset & 0x7FFF) / 32767.0f;
	state->fogStep = (GLfloat)(0x0400 >> engine.renderState.fogShift) / 32767.0f;
	
	for (size_t i = 0; i < 32; i++)
	{
		state->fogDensity[i].r = (engine.renderState.fogDensityTable[i] == 127) ? 1.0f : (GLfloat)engine.renderState.fogDensityTable[i] / 128.0f;
		state->fogDensity[i].g = 0.0f;
		state->fogDensity[i].b = 0.0f;
		state->fogDensity[i].a = 0.0f;
	}
	
	const GLfloat edgeColorAlpha = (engine.renderState.enableAntialiasing) ? (16.0f/31.0f) : 1.0f;
	for (size_t i = 0; i < 8; i++)
	{
		state->edgeColor[i].r = divide5bitBy31_LUT[(engine.renderState.edgeMarkColorTable[i]      ) & 0x001F];
		state->edgeColor[i].g = divide5bitBy31_LUT[(engine.renderState.edgeMarkColorTable[i] >>  5) & 0x001F];
		state->edgeColor[i].b = divide5bitBy31_LUT[(engine.renderState.edgeMarkColorTable[i] >> 10) & 0x001F];
		state->edgeColor[i].a = edgeColorAlpha;
	}
	
	for (size_t i = 0; i < 32; i++)
	{
		state->toonColor[i].r = divide5bitBy31_LUT[(engine.renderState.u16ToonTable[i]      ) & 0x001F];
		state->toonColor[i].g = divide5bitBy31_LUT[(engine.renderState.u16ToonTable[i] >>  5) & 0x001F];
		state->toonColor[i].b = divide5bitBy31_LUT[(engine.renderState.u16ToonTable[i] >> 10) & 0x001F];
		state->toonColor[i].a = 1.0f;
	}
	
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	
	// Do per-poly setup
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_PolyStates);
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboGeometryVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboGeometryIndexID);
	glBindBuffer(GL_TEXTURE_BUFFER, OGLRef.tboPolyStatesID);
	
	size_t vertIndexCount = 0;
	GLushort *indexPtr = (GLushort *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, engine.polylist->count * 6 * sizeof(GLushort), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	OGLPolyStates *polyStates = (OGLPolyStates *)glMapBufferRange(GL_TEXTURE_BUFFER, 0, engine.polylist->count * sizeof(OGLPolyStates), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	
	for (size_t i = 0; i < engine.polylist->count; i++)
	{
		const POLY &thePoly = engine.polylist->list[engine.indexlist.list[i]];
		const size_t polyType = thePoly.type;
		const VERT vert[4] = {
			engine.vertList[thePoly.vertIndexes[0]],
			engine.vertList[thePoly.vertIndexes[1]],
			engine.vertList[thePoly.vertIndexes[2]],
			engine.vertList[thePoly.vertIndexes[3]]
		};
		
		for (size_t j = 0; j < polyType; j++)
		{
			const GLushort vertIndex = thePoly.vertIndexes[j];
			
			// While we're looping through our vertices, add each vertex index to
			// a buffer. For GFX3D_QUADS and GFX3D_QUAD_STRIP, we also add additional
			// vertices here to convert them to GL_TRIANGLES, which are much easier
			// to work with and won't be deprecated in future OpenGL versions.
			indexPtr[vertIndexCount++] = vertIndex;
			if (thePoly.vtxFormat == GFX3D_QUADS || thePoly.vtxFormat == GFX3D_QUAD_STRIP)
			{
				if (j == 2)
				{
					indexPtr[vertIndexCount++] = vertIndex;
				}
				else if (j == 3)
				{
					indexPtr[vertIndexCount++] = thePoly.vertIndexes[0];
				}
			}
		}
		
		// Get the polygon's facing.
		const size_t n = polyType - 1;
		float facing = (vert[0].y + vert[n].y) * (vert[0].x - vert[n].x)
		             + (vert[1].y + vert[0].y) * (vert[1].x - vert[0].x)
		             + (vert[2].y + vert[1].y) * (vert[2].x - vert[1].x);
		
		for (size_t j = 2; j < n; j++)
		{
			facing += (vert[j+1].y + vert[j].y) * (vert[j+1].x - vert[j].x);
		}
		
		this->_isPolyFrontFacing[i] = (facing < 0);
		
		// Get the texture that is to be attached to this polygon.
		this->_textureList[i] = this->GetLoadedTextureFromPolygon(thePoly, this->_enableTextureSampling);
		
		// Get all of the polygon states that can be handled within the shader.
		const NDSTextureFormat packFormat = this->_textureList[i]->GetPackFormat();
		polyStates[i].enableTexture = (this->_textureList[i]->IsSamplingEnabled()) ? GL_TRUE : GL_FALSE;
		polyStates[i].enableFog = (thePoly.attribute.Fog_Enable) ? GL_TRUE : GL_FALSE;
		polyStates[i].isWireframe = (thePoly.isWireframe()) ? GL_TRUE : GL_FALSE;
		polyStates[i].setNewDepthForTranslucent = (thePoly.attribute.TranslucentDepthWrite_Enable) ? GL_TRUE : GL_FALSE;
		polyStates[i].polyAlpha = (thePoly.isWireframe()) ? 0x1F : thePoly.attribute.Alpha;
		polyStates[i].polyMode = thePoly.attribute.Mode;
		polyStates[i].polyID = thePoly.attribute.PolygonID;
		polyStates[i].texSizeS = thePoly.texParam.SizeShiftS; // Note that we are using the preshifted size of S
		polyStates[i].texSizeT = thePoly.texParam.SizeShiftT; // Note that we are using the preshifted size of T
		polyStates[i].texSingleBitAlpha = (packFormat != TEXMODE_A3I5 && packFormat != TEXMODE_A5I3) ? GL_TRUE : GL_FALSE;
	}
	
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glUnmapBuffer(GL_TEXTURE_BUFFER);
	
	const size_t vtxBufferSize = sizeof(VERT) * engine.vertListCount;
	VERT *vtxPtr = (VERT *)glMapBufferRange(GL_ARRAY_BUFFER, 0, vtxBufferSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(vtxPtr, engine.vertList, vtxBufferSize);
	glUnmapBuffer(GL_ARRAY_BUFFER);
	
	glUseProgram(OGLRef.programGeometryID);
	glUniform1i(OGLRef.uniformTexDrawOpaque, GL_FALSE);
	glUniform1i(OGLRef.uniformPolyDrawShadow, GL_FALSE);
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	
	this->_needsZeroDstAlphaPass = true;
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::RenderEdgeMarking(const u16 *colorTable, const bool useAntialias)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	// Set up the postprocessing states
	glViewport(0, 0, this->_framebufferWidth, this->_framebufferHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboPostprocessVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboPostprocessIndexID);
	glBindVertexArray(OGLRef.vaoPostprocessStatesID);
	
	if (this->_emulateSpecialZeroAlphaBlending)
	{
		// Pass 1: Determine the pixels with zero alpha
		glDrawBuffer(GL_NONE);
		glDisable(GL_BLEND);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0x40, 0x40);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0x40);
		glDepthMask(GL_FALSE);
		glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);
		
		glUseProgram(OGLRef.programGeometryZeroDstAlphaID);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
		
		// Pass 2: Unblended edge mark colors to zero-alpha pixels
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glUseProgram(OGLRef.programEdgeMarkID);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
		glStencilFunc(GL_NOTEQUAL, 0x40, 0x40);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
		
		// Pass 3: Blended edge mark
		glEnable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	}
	else
	{
		glUseProgram(OGLRef.programEdgeMarkID);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glDisable(GL_STENCIL_TEST);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	}
	
	glBindVertexArray(0);
	
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	this->_lastTextureDrawTarget = OGLTextureUnitID_GColor;
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::RenderFog(const u8 *densityTable, const u32 color, const u32 offset, const u8 shift, const bool alphaOnly)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboPostprocessID);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glUseProgram(OGLRef.programFogID);
	
	glViewport(0, 0, this->_framebufferWidth, this->_framebufferHeight);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	
	glBindBuffer(GL_ARRAY_BUFFER, OGLRef.vboPostprocessVtxID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OGLRef.iboPostprocessIndexID);
	glBindVertexArray(OGLRef.vaoPostprocessStatesID);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
	
	glBindVertexArray(0);
	
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	this->_lastTextureDrawTarget = OGLTextureUnitID_FinalColor;
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::CreateToonTable()
{
	// Do nothing. The toon table is updated in the render states UBO.
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::DestroyToonTable()
{
	// Do nothing. The toon table is updated in the render states UBO.
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::UpdateToonTable(const u16 *toonTableBuffer)
{
	// Do nothing. The toon table is updated in the render states UBO.
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::ClearUsingImage(const u16 *__restrict colorBuffer, const u32 *__restrict depthBuffer, const u8 *__restrict fogBuffer, const u8 *__restrict polyIDBuffer)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	this->UploadClearImage(colorBuffer, depthBuffer, fogBuffer, polyIDBuffer);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, OGLRef.fboClearImageID);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OGLRef.fboRenderID);
	
	// Blit the polygon ID buffer
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, GPU_FRAMEBUFFER_NATIVE_HEIGHT, GPU_FRAMEBUFFER_NATIVE_WIDTH, 0, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	
	// Blit the fog buffer
	glReadBuffer(GL_COLOR_ATTACHMENT2);
	glDrawBuffer(GL_COLOR_ATTACHMENT2);
	glBlitFramebuffer(0, GPU_FRAMEBUFFER_NATIVE_HEIGHT, GPU_FRAMEBUFFER_NATIVE_WIDTH, 0, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	
	// Blit the color and depth/stencil buffers. Do this last so that color attachment 0 is set to the read FBO.
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, GPU_FRAMEBUFFER_NATIVE_HEIGHT, GPU_FRAMEBUFFER_NATIVE_WIDTH, 0, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboRenderID);
	glDrawBuffers(3, RenderDrawList);
	
	OGLRef.selectedRenderingFBO = (this->_enableMultisampledRendering) ? OGLRef.fboMSIntermediateRenderID : OGLRef.fboRenderID;
	if (OGLRef.selectedRenderingFBO == OGLRef.fboMSIntermediateRenderID)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, OGLRef.fboRenderID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OGLRef.selectedRenderingFBO);
		
		// Blit the polygon ID buffer
		glReadBuffer(GL_COLOR_ATTACHMENT1);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		// Blit the fog buffer
		glReadBuffer(GL_COLOR_ATTACHMENT2);
		glDrawBuffer(GL_COLOR_ATTACHMENT2);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		
		// Blit the color and depth/stencil buffers. Do this last so that color attachment 0 is set to the read FBO.
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, this->_framebufferWidth, this->_framebufferHeight, 0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
		
		glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.selectedRenderingFBO);
		glDrawBuffers(3, RenderDrawList);
	}
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::ClearUsingValues(const FragmentColor &clearColor6665, const FragmentAttributes &clearAttributes)
{
	OGLRenderRef &OGLRef = *this->ref;
	OGLRef.selectedRenderingFBO = (this->_enableMultisampledRendering) ? OGLRef.fboMSIntermediateRenderID : OGLRef.fboRenderID;
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.selectedRenderingFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffers(3, RenderDrawList);
	
	const GLfloat oglColor[4] = {divide6bitBy63_LUT[clearColor6665.r], divide6bitBy63_LUT[clearColor6665.g], divide6bitBy63_LUT[clearColor6665.b], divide5bitBy31_LUT[clearColor6665.a]};
	const GLfloat oglPolyID[4] = {(GLfloat)clearAttributes.opaquePolyID/63.0f, 0.0f, 0.0f, 1.0f};
	const GLfloat oglFogAttr[4] = {(GLfloat)clearAttributes.isFogged, 0.0f, 0.0f, 1.0f};
	
	glClearBufferfi(GL_DEPTH_STENCIL, 0, (GLfloat)clearAttributes.depth / (GLfloat)0x00FFFFFF, clearAttributes.opaquePolyID);
	glClearBufferfv(GL_COLOR, 0, oglColor); // texGColorID
	glClearBufferfv(GL_COLOR, 1, oglPolyID); // texGPolyID
	glClearBufferfv(GL_COLOR, 2, oglFogAttr); // texGFogAttrID
	
	this->_needsZeroDstAlphaPass = (clearColor6665.a == 0);
	
	return OGLERROR_NOERR;
}

void OpenGLRenderer_3_2::SetPolygonIndex(const size_t index)
{
	this->_currentPolyIndex = index;
	glUniform1i(this->ref->uniformPolyStateIndex, index);
}

Render3DError OpenGLRenderer_3_2::SetupPolygon(const POLY &thePoly, bool treatAsTranslucent, bool willChangeStencilBuffer)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	// Set up depth test mode
	glDepthFunc((thePoly.attribute.DepthEqualTest_Enable) ? GL_EQUAL : GL_LESS);
	glUniform1i(OGLRef.uniformPolyDepthOffsetMode, 0);
	
	// Set up culling mode
	static const GLenum oglCullingMode[4] = {GL_FRONT_AND_BACK, GL_FRONT, GL_BACK, 0};
	GLenum cullingMode = oglCullingMode[thePoly.attribute.SurfaceCullingMode];
	
	if (cullingMode == 0)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
		glCullFace(cullingMode);
	}
	
	if (willChangeStencilBuffer)
	{
		// Handle drawing states for the polygon
		if (thePoly.attribute.Mode == POLYGON_MODE_SHADOW)
		{
			if (this->_emulateShadowPolygon)
			{
				// Set up shadow polygon states.
				//
				// See comments in DrawShadowPolygon() for more information about
				// how this 5-pass process works in OpenGL.
				if (thePoly.attribute.PolygonID == 0)
				{
					// 1st pass: Use stencil buffer bit 7 (0x80) for the shadow volume mask.
					// Write only on depth-fail.
					glStencilFunc(GL_ALWAYS, 0x80, 0x80);
					glStencilOp(GL_KEEP, GL_REPLACE, GL_KEEP);
					glStencilMask(0x80);
				}
				else
				{
					// 2nd pass: Compare stencil buffer bits 0-5 (0x3F) with this polygon's ID. If this stencil
					// test fails, remove the fragment from the shadow volume mask by clearing bit 7.
					glStencilFunc(GL_NOTEQUAL, thePoly.attribute.PolygonID, 0x3F);
					glStencilOp(GL_ZERO, GL_KEEP, GL_KEEP);
					glStencilMask(0x80);
				}
				
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
				glDepthMask(GL_FALSE);
			}
		}
		else
		{
			// Polygon IDs are always written for every polygon, whether they are opaque or transparent, just as
			// long as they pass the stencil and depth tests.
			// - Polygon IDs are contained in stencil bits 0-5 (0x3F).
			// - The translucent fragment flag is contained in stencil bit 6 (0x40).
			//
			// Opaque polygons have no stencil conditions, so if they pass the depth test, then they write out
			// their polygon ID with a translucent fragment flag of 0.
			//
			// Transparent polygons have the stencil condition where they will not draw if they are drawing on
			// top of previously drawn translucent fragments with the same polygon ID. This condition is checked
			// using both polygon ID bits and the translucent fragment flag. If the polygon passes both stencil
			// and depth tests, it writes out its polygon ID with a translucent fragment flag of 1.
			if (treatAsTranslucent)
			{
				glStencilFunc(GL_NOTEQUAL, 0x40 | thePoly.attribute.PolygonID, 0x7F);
			}
			else
			{
				glStencilFunc(GL_ALWAYS, thePoly.attribute.PolygonID, 0x3F);
			}
			
			glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
			glStencilMask(0xFF); // Drawing non-shadow polygons will implicitly reset the shadow volume mask.
			
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
			glDepthMask((!treatAsTranslucent || thePoly.attribute.TranslucentDepthWrite_Enable) ? GL_TRUE : GL_FALSE);
		}
	}
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::SetupTexture(const POLY &thePoly, size_t polyRenderIndex)
{
	OpenGLTexture *theTexture = (OpenGLTexture *)this->_textureList[polyRenderIndex];
		
	// Check if we need to use textures
	if (!theTexture->IsSamplingEnabled())
	{
		return OGLERROR_NOERR;
	}
	
	glBindTexture(GL_TEXTURE_2D, theTexture->GetID());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ((thePoly.texParam.RepeatS_Enable) ? ((thePoly.texParam.MirroredRepeatS_Enable) ? GL_MIRRORED_REPEAT : GL_REPEAT) : GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ((thePoly.texParam.RepeatT_Enable) ? ((thePoly.texParam.MirroredRepeatT_Enable) ? GL_MIRRORED_REPEAT : GL_REPEAT) : GL_CLAMP_TO_EDGE));
	
	if (this->_enableTextureSmoothing)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (this->_textureScalingFactor > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, this->_deviceInfo.maxAnisotropy);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
	}
	
	theTexture->ResetCacheAge();
	theTexture->IncreaseCacheUsageCount(1);
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::SetFramebufferSize(size_t w, size_t h)
{
	OGLRenderRef &OGLRef = *this->ref;
	
	if (w < GPU_FRAMEBUFFER_NATIVE_WIDTH || h < GPU_FRAMEBUFFER_NATIVE_HEIGHT)
	{
		return OGLERROR_NOERR;
	}
	
	if (!BEGINGL())
	{
		return OGLERROR_BEGINGL_FAILED;
	}
	
	glFinish();
	
	if (this->_mappedFramebuffer != NULL)
	{
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		glFinish();
	}
	
	const size_t newFramebufferColorSizeBytes = w * h * sizeof(FragmentColor);
	glBufferData(GL_PIXEL_PACK_BUFFER, newFramebufferColorSizeBytes, NULL, GL_STREAM_READ);
	
	if (this->_mappedFramebuffer != NULL)
	{
		this->_mappedFramebuffer = (FragmentColor *__restrict)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		glFinish();
	}
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_FinalColor);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texFinalColorID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_DepthStencil);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texGDepthStencilID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_GColor);
	glBindTexture(GL_TEXTURE_2D, OGLRef.texGColorID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_GPolyID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0 + OGLTextureUnitID_FogAttr);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	
	glActiveTexture(GL_TEXTURE0);
	
	this->_framebufferWidth = w;
	this->_framebufferHeight = h;
	this->_framebufferPixCount = w * h;
	this->_framebufferColorSizeBytes = newFramebufferColorSizeBytes;
	this->_framebufferColor = NULL; // Don't need to make a client-side buffer since we will be reading directly from the PBO.
	
	// Call ResizeMultisampledFBOs() after _framebufferWidth and _framebufferHeight are set
	// since this method depends on them.
	GLsizei sampleSize = this->GetLimitedMultisampleSize();
	this->ResizeMultisampledFBOs(sampleSize);
	
	if (oglrender_framebufferDidResizeCallback != NULL)
	{
		oglrender_framebufferDidResizeCallback(w, h);
	}
	
	glFinish();
	ENDGL();
	
	return OGLERROR_NOERR;
}

Render3DError OpenGLRenderer_3_2::RenderPowerOff()
{
	OGLRenderRef &OGLRef = *this->ref;
	static const GLfloat oglColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	
	if (!this->_isPoweredOn)
	{
		return OGLERROR_NOERR;
	}
	
	this->_isPoweredOn = false;
	memset(GPU->GetEngineMain()->Get3DFramebufferMain(), 0, this->_framebufferColorSizeBytes);
	memset(GPU->GetEngineMain()->Get3DFramebuffer16(), 0, this->_framebufferPixCount * sizeof(u16));
	
	if(!BEGINGL())
	{
		return OGLERROR_BEGINGL_FAILED;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, OGLRef.fboRenderID);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClearBufferfv(GL_COLOR, 0, oglColor);
	
	if (this->_mappedFramebuffer != NULL)
	{
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
		this->_mappedFramebuffer = NULL;
	}
	
	glReadPixels(0, 0, this->_framebufferWidth, this->_framebufferHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
	
	ENDGL();
	
	this->_pixelReadNeedsFinish = true;
	return OGLERROR_NOERR;
}
