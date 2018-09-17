#include "VkGLSL.hpp"
#include <boost\algorithm\string.hpp>

namespace ASGI { namespace VKGLSL {
	std::unordered_map<GLSLType, std::string>  GLSLTypeStrMap = {
	{ GLSLType::GLSL_TYPE_VOID, "void" },
	{ GLSLType::GLSL_TYPE_BOOL, "bool" },
	{ GLSLType::GLSL_TYPE_BVEC2, "bvec2" },
	{ GLSLType::GLSL_TYPE_BVEC3, "bvec3" },
	{ GLSLType::GLSL_TYPE_BVEC4, "bvec4" },
	{ GLSLType::GLSL_TYPE_INT, "int" },
	{ GLSLType::GLSL_TYPE_IVEC2, "ivec2" },
	{ GLSLType::GLSL_TYPE_IVEC3, "ivec3" },
	{ GLSLType::GLSL_TYPE_IVEC4, "ivec4" },
	{ GLSLType::GLSL_TYPE_UINT, "uint" },
	{ GLSLType::GLSL_TYPE_UVEC2, "uvec2" },
	{ GLSLType::GLSL_TYPE_UVEC3, "uvec3" },
	{ GLSLType::GLSL_TYPE_UVEC4, "uvec4" },
	{ GLSLType::GLSL_TYPE_FLOAT, "float" },
	{ GLSLType::GLSL_TYPE_VEC2, "vec2" },
	{ GLSLType::GLSL_TYPE_VEC3, "vec3" },
	{ GLSLType::GLSL_TYPE_VEC4, "vec4" },
	{ GLSLType::GLSL_TYPE_DOUBLE, "double" },
	{ GLSLType::GLSL_TYPE_DVEC2, "dvec2" },
	{ GLSLType::GLSL_TYPE_DEVC3, "dvec3" },
	{ GLSLType::GLSL_TYPE_DEVC4, "dvec4" },
	{ GLSLType::GLSL_TYPE_MAT2, "mat2" },
	{ GLSLType::GLSL_TYPE_MAT3, "mat3" },
	{ GLSLType::GLSL_TYPE_MAT4, "mat4" },
	{ GLSLType::GLSL_TYPE_MAT2X3, "mat2x3" },
	{ GLSLType::GLSL_TYPE_MAT2X4, "mat2x4" },
	{ GLSLType::GLSL_TYPE_MAT3X2, "mat3x2" },
	{ GLSLType::GLSL_TYPE_MAT4X2, "mat4x2" },
	{ GLSLType::GLSL_TYPE_MAT3X4, "mat3x4" },
	{ GLSLType::GLSL_TYPE_MAT4X3, "mat4x3" },
	{ GLSLType::GLSL_TYPE_DMAT2, "dmat2" },
	{ GLSLType::GLSL_TYPE_DMAT3, "dmat3" },
	{ GLSLType::GLSL_TYPE_DMAT4, "dmat4" },
	{ GLSLType::GLSL_TYPE_DMAT2X3, "dmat2x3" },
	{ GLSLType::GLSL_TYPE_DMAT2X4, "dmat2x4" },
	{ GLSLType::GLSL_TYPE_DMAT3X2, "dmat3x2" },
	{ GLSLType::GLSL_TYPE_DMAT4X2, "dmat4x2" },
	{ GLSLType::GLSL_TYPE_DMAT3X4, "dmat3x4" },
	{ GLSLType::GLSL_TYPE_DMAT4X3, "dmat4x3" },
	{ GLSLType::GLSL_TYPE_SAMPLER1D, "sampler1D" },
	{ GLSLType::GLSL_TYPE_ISAMPLER1D, "isampler1D" },
	{ GLSLType::GLSL_TYPE_USAMPLER1D, "usampler1D" },
	{ GLSLType::GLSL_TYPE_IMAGE1D, "sampler1D" },
	{ GLSLType::GLSL_TYPE_IIMAGE1D, "iimage1D" },
	{ GLSLType::GLSL_TYPE_UIMAGE1D, "uimage1D" },
	{ GLSLType::GLSL_TYPE_SAMPLER1D_ARRAY, "sampler1DArray" },
	{ GLSLType::GLSL_TYPE_ISAMPLER1D_ARRAY, "isampler1DArray" },
	{ GLSLType::GLSL_TYPE_USAMPLER1D_ARRAY, "usampler1DArray" },
	{ GLSLType::GLSL_TYPE_IMAGE1D_ARRAY, "image1DArray" },
	{ GLSLType::GLSL_TYPE_IIMAGE1D_ARRAY, "iimage1DArray" },
	{ GLSLType::GLSL_TYPE_UIMAGE1D_ARRAY, "uimage1DArray" },
	{ GLSLType::GLSL_TYPE_SAMPLER1D_ARRAY_SHADOW, "sampler1DArrayShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER1D_SHADOW, "sampler1DShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER2D, "sampler2D" },
	{ GLSLType::GLSL_TYPE_ISAMPLER2D, "isampler2D" },
	{ GLSLType::GLSL_TYPE_USAMPLER2D, "usampler2D" },
	{ GLSLType::GLSL_TYPE_IMAGE2D, "image2D" },
	{ GLSLType::GLSL_TYPE_IIMAGE2D, "iimage2D" },
	{ GLSLType::GLSL_TYPE_UIMAGE2D, "uimage2D" },
	{ GLSLType::GLSL_TYPE_SAMPLER2D_ARRAY, "sampler2DArray" },
	{ GLSLType::GLSL_TYPE_ISAMPLER2D_ARRAY, "isampler2DArray" },
	{ GLSLType::GLSL_TYPE_USAMPLER2D_ARRAY, "usampler2DArray" },
	{ GLSLType::GLSL_TYPE_IMAGE2D_ARRAY, "image2DArray" },
	{ GLSLType::GLSL_TYPE_IIMAGE2D_ARRAY, "iimage2DArray" },
	{ GLSLType::GLSL_TYPE_UIMAGE2D_ARRAY, "uimage2DArray" },
	{ GLSLType::GLSL_TYPE_SAMPLER2D_ARRAY_SHADOW, "sampler2DArrayShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER2D_RECT, "sampler2DRect" },
	{ GLSLType::GLSL_TYPE_ISAMPLER2D_RECT, "isampler2DRect" },
	{ GLSLType::GLSL_TYPE_USAMPLER2D_RECT, "usampler2DRect" },
	{ GLSLType::GLSL_TYPE_IMAGE2D_RECT, "image2DRect" },
	{ GLSLType::GLSL_TYPE_IIMAGE2D_RECT, "iimage2DRect" },
	{ GLSLType::GLSL_TYPE_UIMAGE2D_RECT, "uimage2DRect" },
	{ GLSLType::GLSL_TYPE_SAMPLER2D_RECT_SHADOW, "sampler2DRectShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER2D_SHADOW, "sampler2DShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER3D, "sampler3D" },
	{ GLSLType::GLSL_TYPE_ISAMPLER3D, "isampler3D" },
	{ GLSLType::GLSL_TYPE_USAMPLER3D, "usampler3D" },
	{ GLSLType::GLSL_TYPE_IMAGE3D, "image3D" },
	{ GLSLType::GLSL_TYPE_IIMAGE3D, "iimage3D" },
	{ GLSLType::GLSL_TYPE_UIMAGE3D, "uimage3D" },
	{ GLSLType::GLSL_TYPE_SAMPLER_2DMS, "sampler2DMS" },
	{ GLSLType::GLSL_TYPE_ISAMPLER_2DMS, "isampler2DMS" },
	{ GLSLType::GLSL_TYPE_USAMPLER_2DMS, "usampler2DMS" },
	{ GLSLType::GLSL_TYPE_IMAGE_2DMS, "image2DMS" },
	{ GLSLType::GLSL_TYPE_IIMAGE_2DMS, "iimage2DMS" },
	{ GLSLType::GLSL_TYPE_UIMAGE_2DMS, "uimage2DMS" },
	{ GLSLType::GLSL_TYPE_SAMPLER_2DMS_ARRAY, "sampler2DMSArray" },
	{ GLSLType::GLSL_TYPE_ISAMPLER_2DMS_ARRAY, "isampler2DMSArray" },
	{ GLSLType::GLSL_TYPE_USAMPLER_2DMS_ARRAY, "usampler2DMSArray" },
	{ GLSLType::GLSL_TYPE_IMAGE_2DMS_ARRAY, "image2DMSArray" },
	{ GLSLType::GLSL_TYPE_IIMAGE_2DMS_ARRAY, "iimage2DMSArray" },
	{ GLSLType::GLSL_TYPE_UIMAGE_2DMS_ARRAY, "uimage2DMSArray" },
	{ GLSLType::GLSL_TYPE_SAMPLER_CUBE, "samplerCube" },
	{ GLSLType::GLSL_TYPE_ISAMPLER_CUBE, "isamplerCube" },
	{ GLSLType::GLSL_TYPE_USAMPLER_CUBE, "usamplerCube" },
	{ GLSLType::GLSL_TYPE_IMAGE_CUBE, "imageCube" },
	{ GLSLType::GLSL_TYPE_IIMAGE_CUBE, "iimageCube" },
	{ GLSLType::GLSL_TYPE_UIMAGE_CUBE, "uimageCube" },
	{ GLSLType::GLSL_TYPE_SAMPLER_CUBE_ARRAY, "samplerCubeArray" },
	{ GLSLType::GLSL_TYPE_ISAMPLER_CUBE_ARRAY, "isamplerCubeArray" },
	{ GLSLType::GLSL_TYPE_USAMPLER_CUBE_ARRAY, "usamplerCubeArray" },
	{ GLSLType::GLSL_TYPE_IMAGE_CUBE_ARRAY, "imageCubeArray" },
	{ GLSLType::GLSL_TYPE_IIMAGE_CUBE_ARRAY, "iimageCubeArray" },
	{ GLSLType::GLSL_TYPE_UIMAGE_CUBE_ARRAY, "uimageCubeArray" },
	{ GLSLType::GLSL_TYPE_SAMPLER_CUBE_ARRAY_SHADOW, "samplerCubeArrayShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER_CUBE_SHADOW, "samplerCubeShadow" },
	{ GLSLType::GLSL_TYPE_SAMPLER_BUFFER, "samplerBuffer" },
	{ GLSLType::GLSL_TYPE_ISAMPLER_BUFFER, "isamplerBuffer" },
	{ GLSLType::GLSL_TYPE_USAMPLER_BUFFER, "usamplerBuffer" },
	{ GLSLType::GLSL_TYPE_IMAGE_BUFFER, "imageBuffer" },
	{ GLSLType::GLSL_TYPE_IIMAGE_BUFFER, "iimageBuffer" },
	{ GLSLType::GLSL_TYPE_UIMAGE_BUFFER, "uimageBuffer" },
	{ GLSLType::GLSL_TYPE_ATOMIC_UINT, "atomic_uint" }
	};

	std::unordered_map<std::string, GLSLType>  GLSLStrTypeMap = {
	{ "void", GLSLType::GLSL_TYPE_VOID },
	{ "bool",  GLSLType::GLSL_TYPE_BOOL },
	{ "bvec2" , GLSLType::GLSL_TYPE_BVEC2 },
	{ "bvec3",  GLSLType::GLSL_TYPE_BVEC3 },
	{ "bvec4",  GLSLType::GLSL_TYPE_BVEC4 },
	{ "int" , GLSLType::GLSL_TYPE_INT },
	{ "ivec2",  GLSLType::GLSL_TYPE_IVEC2 },
	{ "ivec3" , GLSLType::GLSL_TYPE_IVEC3 },
	{ "ivec4" , GLSLType::GLSL_TYPE_IVEC4 },
	{ "uint", GLSLType::GLSL_TYPE_UINT },
	{ "uvec2" , GLSLType::GLSL_TYPE_UVEC2 },
	{ "uvec3" , GLSLType::GLSL_TYPE_UVEC3 },
	{ "uvec4" , GLSLType::GLSL_TYPE_UVEC4 },
	{ "float",  GLSLType::GLSL_TYPE_FLOAT },
	{ "vec2",  GLSLType::GLSL_TYPE_VEC2 },
	{ "vec3" , GLSLType::GLSL_TYPE_VEC3 },
	{ "vec4" , GLSLType::GLSL_TYPE_VEC4 },
	{ "double",  GLSLType::GLSL_TYPE_DOUBLE },
	{ "dvec2",  GLSLType::GLSL_TYPE_DVEC2 },
	{ "dvec3",  GLSLType::GLSL_TYPE_DEVC3 },
	{ "dvec4" , GLSLType::GLSL_TYPE_DEVC4 },
	{ "mat2" , GLSLType::GLSL_TYPE_MAT2 },
	{ "mat3" , GLSLType::GLSL_TYPE_MAT3 },
	{ "mat4",  GLSLType::GLSL_TYPE_MAT4 },
	{ "mat2x3",  GLSLType::GLSL_TYPE_MAT2X3 },
	{ "mat2x4" , GLSLType::GLSL_TYPE_MAT2X4 },
	{ "mat3x2",  GLSLType::GLSL_TYPE_MAT3X2 },
	{ "mat4x2",  GLSLType::GLSL_TYPE_MAT4X2 },
	{ "mat3x4" , GLSLType::GLSL_TYPE_MAT3X4 },
	{ "mat4x3",  GLSLType::GLSL_TYPE_MAT4X3 },
	{ "dmat2" , GLSLType::GLSL_TYPE_DMAT2 },
	{ "dmat3",  GLSLType::GLSL_TYPE_DMAT3 },
	{ "dmat4",  GLSLType::GLSL_TYPE_DMAT4 },
	{ "dmat2x3" , GLSLType::GLSL_TYPE_DMAT2X3 },
	{ "dmat2x4",  GLSLType::GLSL_TYPE_DMAT2X4 },
	{ "dmat3x2",  GLSLType::GLSL_TYPE_DMAT3X2 },
	{ "dmat4x2" , GLSLType::GLSL_TYPE_DMAT4X2 },
	{ "dmat3x4",  GLSLType::GLSL_TYPE_DMAT3X4 },
	{ "dmat4x3",  GLSLType::GLSL_TYPE_DMAT4X3 },
	{ "sampler1D",  GLSLType::GLSL_TYPE_SAMPLER1D },
	{ "isampler1D",  GLSLType::GLSL_TYPE_ISAMPLER1D },
	{ "usampler1D",  GLSLType::GLSL_TYPE_USAMPLER1D },
	{ "image1D",  GLSLType::GLSL_TYPE_IMAGE1D },
	{ "iimage1D",  GLSLType::GLSL_TYPE_IIMAGE1D },
	{ "uimage1D",  GLSLType::GLSL_TYPE_UIMAGE1D },
	{ "sampler1DArray" , GLSLType::GLSL_TYPE_SAMPLER1D_ARRAY },
	{ "isampler1DArray" , GLSLType::GLSL_TYPE_ISAMPLER1D_ARRAY },
	{ "usampler1DArray" , GLSLType::GLSL_TYPE_USAMPLER1D_ARRAY },
	{ "image1DArray" , GLSLType::GLSL_TYPE_IMAGE1D_ARRAY },
	{ "iimage1DArray" , GLSLType::GLSL_TYPE_IIMAGE1D_ARRAY },
	{ "uimage1DArray" , GLSLType::GLSL_TYPE_UIMAGE1D_ARRAY },
	{ "sampler1DArrayShadow" , GLSLType::GLSL_TYPE_SAMPLER1D_ARRAY_SHADOW },
	{ "sampler1DShadow",  GLSLType::GLSL_TYPE_SAMPLER1D_SHADOW },
	{ "sampler2D" , GLSLType::GLSL_TYPE_SAMPLER2D },
	{ "isampler2D" , GLSLType::GLSL_TYPE_ISAMPLER2D },
	{ "usampler2D" , GLSLType::GLSL_TYPE_USAMPLER2D },
	{ "image2D" , GLSLType::GLSL_TYPE_IMAGE2D },
	{ "iimage2D" , GLSLType::GLSL_TYPE_IIMAGE2D },
	{ "uimage2D" , GLSLType::GLSL_TYPE_UIMAGE2D },
	{ "sampler2DArray",  GLSLType::GLSL_TYPE_SAMPLER2D_ARRAY },
	{ "isampler2DArray",  GLSLType::GLSL_TYPE_ISAMPLER2D_ARRAY },
	{ "usampler2DArray",  GLSLType::GLSL_TYPE_USAMPLER2D_ARRAY },
	{ "image2DArray",  GLSLType::GLSL_TYPE_IMAGE2D_ARRAY },
	{ "iimage2DArray",  GLSLType::GLSL_TYPE_IIMAGE2D_ARRAY },
	{ "uimage2DArray",  GLSLType::GLSL_TYPE_UIMAGE2D_ARRAY },
	{ "sampler2DArrayShadow",  GLSLType::GLSL_TYPE_SAMPLER2D_ARRAY_SHADOW },
	{ "sampler2DRect", GLSLType::GLSL_TYPE_SAMPLER2D_RECT },
	{ "isampler2DRect", GLSLType::GLSL_TYPE_ISAMPLER2D_RECT },
	{ "usampler2DRect", GLSLType::GLSL_TYPE_USAMPLER2D_RECT },
	{ "image2DRect", GLSLType::GLSL_TYPE_IMAGE2D_RECT },
	{ "iimage2DRect", GLSLType::GLSL_TYPE_IIMAGE2D_RECT },
	{ "uimage2DRect", GLSLType::GLSL_TYPE_UIMAGE2D_RECT },
	{ "sampler2DRectShadow" , GLSLType::GLSL_TYPE_SAMPLER2D_RECT_SHADOW },
	{ "sampler2DShadow",  GLSLType::GLSL_TYPE_SAMPLER2D_SHADOW },
	{ "sampler3D" , GLSLType::GLSL_TYPE_SAMPLER3D },
	{ "isampler3D" , GLSLType::GLSL_TYPE_ISAMPLER3D },
	{ "usampler3D" , GLSLType::GLSL_TYPE_USAMPLER3D },
	{ "image3D" , GLSLType::GLSL_TYPE_IMAGE3D },
	{ "iimage3D" , GLSLType::GLSL_TYPE_IIMAGE3D },
	{ "uimage3D" , GLSLType::GLSL_TYPE_UIMAGE3D },
	{ "sampler2DMS" , GLSLType::GLSL_TYPE_SAMPLER_2DMS },
	{ "isampler2DMS" , GLSLType::GLSL_TYPE_ISAMPLER_2DMS },
	{ "usampler2DMS" , GLSLType::GLSL_TYPE_USAMPLER_2DMS },
	{ "image2DMS" , GLSLType::GLSL_TYPE_IMAGE_2DMS },
	{ "iimage2DMS" , GLSLType::GLSL_TYPE_IIMAGE_2DMS },
	{ "uimage2DMS" , GLSLType::GLSL_TYPE_UIMAGE_2DMS },
	{ "sampler2DMSArray" , GLSLType::GLSL_TYPE_SAMPLER_2DMS_ARRAY },
	{ "isampler2DMSArray" , GLSLType::GLSL_TYPE_ISAMPLER_2DMS_ARRAY },
	{ "usampler2DMSArray" , GLSLType::GLSL_TYPE_USAMPLER_2DMS_ARRAY },
	{ "image2DMSArray" , GLSLType::GLSL_TYPE_IMAGE_2DMS_ARRAY },
	{ "iimage2DMSArray" , GLSLType::GLSL_TYPE_IIMAGE_2DMS_ARRAY },
	{ "uimage2DMSArray" , GLSLType::GLSL_TYPE_UIMAGE_2DMS_ARRAY },
	{ "samplerCube", GLSLType::GLSL_TYPE_SAMPLER_CUBE },
	{ "isamplerCube", GLSLType::GLSL_TYPE_ISAMPLER_CUBE },
	{ "usamplerCube", GLSLType::GLSL_TYPE_USAMPLER_CUBE },
	{ "imageCube", GLSLType::GLSL_TYPE_IMAGE_CUBE },
	{ "iimageCube", GLSLType::GLSL_TYPE_IIMAGE_CUBE },
	{ "uimageCube", GLSLType::GLSL_TYPE_UIMAGE_CUBE },
	{ "samplerCubeArray" , GLSLType::GLSL_TYPE_SAMPLER_CUBE_ARRAY },
	{ "isamplerCubeArray" , GLSLType::GLSL_TYPE_ISAMPLER_CUBE_ARRAY },
	{ "usamplerCubeArray" , GLSLType::GLSL_TYPE_USAMPLER_CUBE_ARRAY },
	{ "imageCubeArray" , GLSLType::GLSL_TYPE_IMAGE_CUBE_ARRAY },
	{ "iiamgeCubeArray" , GLSLType::GLSL_TYPE_IIMAGE_CUBE_ARRAY },
	{ "uimageCubeArray" , GLSLType::GLSL_TYPE_UIMAGE_CUBE_ARRAY },
	{ "samplerCubeArrayShadow" , GLSLType::GLSL_TYPE_SAMPLER_CUBE_ARRAY_SHADOW },
	{ "samplerCubeShadow" , GLSLType::GLSL_TYPE_SAMPLER_CUBE_SHADOW },
	{ "samplerBuffer",  GLSLType::GLSL_TYPE_SAMPLER_BUFFER },
	{ "isamplerBuffer",  GLSLType::GLSL_TYPE_ISAMPLER_BUFFER },
	{ "usamplerBuffer",  GLSLType::GLSL_TYPE_USAMPLER_BUFFER },
	{ "imageBuffer",  GLSLType::GLSL_TYPE_IMAGE_BUFFER },
	{ "iimageBuffer",  GLSLType::GLSL_TYPE_IIMAGE_BUFFER },
	{ "uimageBuffer",  GLSLType::GLSL_TYPE_UIMAGE_BUFFER },
	{ "atomic_uint" , GLSLType::GLSL_TYPE_ATOMIC_UINT }
	};


	std::unordered_map<std::string, GLSLStorageQualifier> GLSLStrSQFMap = {
	{"const", GLSLStorageQualifier::GLSL_SQ_CONST},
	{"in", GLSLStorageQualifier::GLSL_SQ_IN},
	{"out", GLSLStorageQualifier::GLSL_SQ_OUT},
	{"uniform", GLSLStorageQualifier::GLSL_SQ_UNIFORM},
	{"buffer", GLSLStorageQualifier::GLSL_SQ_BUFFER},
	{"shared", GLSLStorageQualifier::GLSL_SQ_SHARED},
	{"attribute", GLSLStorageQualifier::GLSL_SQ_ATTRIBUTE},
	{"varying", GLSLStorageQualifier::GLSL_SQ_VARYING}
	};

	inline bool is_azAz(const char t) {
		if ((t >= 97 && t <= 122) || (t >= 65 && t <= 90) || t == 95) {
			return true;
		}
		//
		return false;
	}
	//
	bool preprocess(const char* pcode, std::string& result) {
		std::vector<char> tmp(strlen(pcode) + 1, '\0');
		uint32_t currentP = 0;
		//
		char* processingCode = const_cast<char*>(pcode);
		//
		while (true) {
			int32_t codeSize = strlen(processingCode);
			if (codeSize <= 0) {
				result.append(tmp.data(), currentP + 1);
				break;
			}
			//
			if (strncmp(processingCode, "\\\n", 2) == 0) {
				processingCode = processingCode + 2;
				continue;
			}
			else if (strncmp(processingCode, "//", 2) == 0) {
				uint32_t endPos = 1;
				while (endPos < codeSize && (processingCode[endPos] != '\n' || processingCode[endPos - 1] == '\\')) {
					++endPos;
				}
				processingCode = processingCode + endPos + 1;
				continue;
			}
			else if (strncmp(processingCode, "/*", 2) == 0) {
				uint32_t p = 2;
				while (true) {
					if (processingCode[p] == '*' && processingCode[p + 1] == '/') {
						processingCode = processingCode + p + 2;
						break;
					}
					//
					if (++p >= codeSize) {
						return false;
					}
				}
			}
			else {
				tmp[currentP++] = *processingCode;
				processingCode++;
			}
		}
		//
		return true;
	}
	//
	bool analyzeStruct(const char* pcode, uint32_t code_size, GLSLLayout& layout) {
		std::string structDefine = std::string(pcode, code_size);
		//
		GLSLStruct glslStruct;
		//
		glslStruct.name = boost::trim_copy(structDefine.substr(6, structDefine.find_first_of('{') - 6));
		std::string memberList = std::string(pcode + structDefine.find_first_of('{') + 1, pcode + structDefine.find_first_of('}'));
		const char* analysingCode = memberList.c_str();
		while (strlen(analysingCode) > 0) {
			uint32_t codeSize = strlen(analysingCode);
			//
			if (analysingCode[0] == ' ' || analysingCode[0] == '\n' || analysingCode[0] == '\t' || analysingCode[0] == '\r') {
				analysingCode++;
				continue;
			}
			else {
				uint32_t p = 0;
				while (p < codeSize && analysingCode[p++] != ' ') {}
				auto tmp = std::string(analysingCode, analysingCode + p - 1);
				bool isTypeName = false;
				if (GLSLStrTypeMap.find(tmp) != GLSLStrTypeMap.end() || layout.structs.find(tmp) != layout.structs.end()) {
					isTypeName = true;
					//
					analysingCode = analysingCode + p;
					while (p < codeSize && analysingCode[p++] != ';') {}
					auto memberName = boost::trim_copy(std::string(analysingCode, analysingCode + p - 1));
					//
					glslStruct.defines.push_back(std::pair<std::string, std::string>(memberName, tmp));
					//
					analysingCode = analysingCode + p;
				}
				if (!isTypeName) {
					return false;
				}
			}
		}
		//
		layout.structs[glslStruct.name] = glslStruct;
		//
		return true;
	}

	bool analyzeConstant(const char* pcode, uint32_t code_size, GLSLLayout& layout) {
		GLSLConstant glslConstant;
		//
		auto constDefine = boost::trim_copy(std::string(pcode + 5, pcode + code_size + 1));
		//
		auto strType = constDefine.substr(0, constDefine.find_first_of(' '));
		auto strName = std::string(constDefine.c_str() + strType.length(), constDefine.c_str() + constDefine.find_first_of('='));
		auto strValue = std::string(constDefine.c_str() + constDefine.find_first_of('=') + 1, constDefine.c_str() + constDefine.find_first_of(';'));
		//
		boost::trim(strType);
		auto itr_glsl_type = GLSLStrTypeMap.find(strType);
		if (itr_glsl_type == GLSLStrTypeMap.end()) {
			return false;
		}
		glslConstant.constant_id = -1;
		glslConstant.name = boost::trim_copy(strName);
		glslConstant.type = itr_glsl_type->second;
		glslConstant.strValue = boost::trim_copy(strValue);
		//
		layout.constants[glslConstant.name] = glslConstant;
		//
		return true;
	}

	bool analyzeLayoutWithInSQ(std::string& layoutQualifiers, std::string& define, GLSLLayout& layout) {
		int location = -1;
		if (layoutQualifiers.substr(0, 8) == "location") {
			auto strLocation = std::string(layoutQualifiers.c_str() + layoutQualifiers.find_first_of('=') + 1, layoutQualifiers.c_str() + layoutQualifiers.length());
			location = atoi(boost::trim_copy(strLocation).c_str());
		}
		if (location < 0) {
			return false;
		}
		//
		auto strType = define.substr(0, define.find_first_of(' '));
		auto itrGLSLType = GLSLStrTypeMap.find(strType);
		if (itrGLSLType == GLSLStrTypeMap.end()) {
			return false;
		}
		//
		auto name = boost::trim_copy(std::string(define.c_str() + define.find_first_of(' '), define.c_str() + define.length()));
		//
		GLSLInputAttribute tmp;
		tmp.location = location;
		tmp.name = name;
		tmp.type = itrGLSLType->second;
		layout.inputAttributes.push_back(tmp);
		//
		return true;
	}

	bool analyzeLayoutWithConstSQ(std::string& layoutQualifiers, std::string& define, GLSLLayout& layout) {
		return true;
	}

	bool analyzeLayoutWithUninormSQ(std::string& layoutQualifiers, std::string& define, GLSLLayout& layout) {
		if (define.substr(0, 9) == "sampler2D") {

		}
		return true;
	}

	bool analyzeLayout(const char* pcode, uint32_t code_size, GLSLLayout& layout) {
		auto strDefine = std::string(pcode, code_size);
		auto strLayoutQualifiers = boost::trim_copy(std::string(pcode + strDefine.find_first_of('(') + 1, pcode + strDefine.find_first_of(')')));
		strDefine = boost::trim_copy(std::string(pcode + strDefine.find_first_of(')') + 1, pcode + code_size));
		auto strStorageQualifier = strDefine.substr(0, strDefine.find_first_of(' '));
		auto itrSQF = GLSLStrSQFMap.find(strStorageQualifier);
		if (itrSQF == GLSLStrSQFMap.end()) {
			return false;
		}
		strDefine = boost::trim_copy(std::string(strDefine.c_str() + strStorageQualifier.length(), strDefine.c_str() + strDefine.length()));
		//
		if (itrSQF->second == GLSLStorageQualifier::GLSL_SQ_IN) {
			return analyzeLayoutWithInSQ(strLayoutQualifiers, strDefine, layout);
		}
		else if (itrSQF->second == GLSLStorageQualifier::GLSL_SQ_CONST) {
			return analyzeLayoutWithConstSQ(strLayoutQualifiers, strDefine, layout);
		}
		else if (itrSQF->second == GLSLStorageQualifier::GLSL_SQ_UNIFORM) {
			return analyzeLayoutWithUninormSQ(strLayoutQualifiers, strDefine, layout);
		}
		//
		return false;
	}
	//
	bool GLSLLayoutAnalyze(const char* pcode, GLSLLayout& layout) {
		std::string strCode;
		if (!preprocess(pcode, strCode)) {
			return false;
		}
		//
		const char* analysingCode = strCode.c_str();
		while (strlen(analysingCode) > 0) {
			uint32_t codeSize = strlen(analysingCode);
			//
			if (analysingCode[0] == ' ' || analysingCode[0] == '\n' || analysingCode[0] == '\t' || analysingCode[0] == '\r') {
				analysingCode++;
				continue;
			}
			else if (analysingCode[0] == '#') {
				uint32_t inIF = 0;
				uint32_t p = 1;
				while (true) {
					if (strncmp(analysingCode + p, "if", 2) == 0) {
						++inIF;
					}
					else if (strncmp(analysingCode + p, "endif", 5) == 0) {
						--inIF;
					}
					else if (analysingCode[p] == '\n' && !inIF) {
						analysingCode = analysingCode + p + 1;
						break;
					}
					//
					if (++p >= codeSize) {
						return false;
					}
				}
			}
			else if (strncmp(analysingCode, "layout", 6) == 0 && (analysingCode[6] == ' ' || analysingCode[6] == '(')) {
				uint32_t inBlock = 0;
				uint32_t p = 3;
				while (true) {
					if (analysingCode[p] == '{') {
						++inBlock;
					}
					else if (analysingCode[p] == ';' && !inBlock) {
						if (!analyzeLayout(analysingCode, p, layout)) {
							return false;
						}
						analysingCode = analysingCode + p + 1;
						break;
					}
					else if (analysingCode[p] == '}') {
						--inBlock;
					}
					//
					if (++p >= codeSize) {
						return false;
					}
				}
			}
			else if (strncmp(analysingCode, "struct", 6) == 0 && analysingCode[6] == ' ') {
				uint32_t inBlock = 0;
				uint32_t p = 6;
				//
				std::string structDefine;
				//
				while (true) {
					if (analysingCode[p] == '{') {
						++inBlock;
					}
					else if (analysingCode[p] == ';' && !inBlock) {
						if (!analyzeStruct(analysingCode, p, layout)) {
							return false;
						}
						analysingCode = analysingCode + p + 1;
						break;
					}
					else if (analysingCode[p] == '}') {
						--inBlock;
					}
					//
					if (++p >= codeSize) {
						return false;
					}
				}
			}
			else if (strncmp(analysingCode, "const", 5) == 0 && analysingCode[5] == ' ') {
				uint32_t inBlock = 0;
				uint32_t p = 5;
				while (true) {
					if (analysingCode[p] == ';') {
						if (!analyzeConstant(analysingCode, p, layout)) {
							return false;
						}
						analysingCode = analysingCode + p + 1;
						break;
					}
					//
					if (++p >= codeSize) {
						return false;
					}
				}
			}
			else {
				analysingCode++;
			}
		}
		//
		return true;
	}
}}