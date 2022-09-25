#ifndef __MQO_H__
#define __MQO_H__

#define DEFINE_MQO_DEF_HEADER_FORMAT       ("Text")
#define DEFINE_MQO_DEF_HEADER_VERSION      (1.1f)
#define DEFINE_MQO_DEF_HEADER_CODEPAGE     ("932")

#define DEFINE_MQO_DEF_SCENE_POS_X         (5)
#define DEFINE_MQO_DEF_SCENE_POS_Y         (-15)
#define DEFINE_MQO_DEF_SCENE_POS_Z         (400)
#define DEFINE_MQO_DEF_SCENE_LOOKAT_X      (0)
#define DEFINE_MQO_DEF_SCENE_LOOKAT_Y      (0)
#define DEFINE_MQO_DEF_SCENE_LOOKAT_Z      (0)
#define DEFINE_MQO_DEF_SCENE_HEAD          (0.0f)
#define DEFINE_MQO_DEF_SCENE_PICH          (1.5708f)
#define DEFINE_MQO_DEF_SCENE_BANK          (0.0f)
#define DEFINE_MQO_DEF_SCENE_ORTHO         (0)
#define DEFINE_MQO_DEF_SCENE_ZOOM2         (10.0f)
#define DEFINE_MQO_DEF_SCENE_AMB_X         (0.25f)
#define DEFINE_MQO_DEF_SCENE_AMB_Y         (0.25f)
#define DEFINE_MQO_DEF_SCENE_AMB_Z         (0.25f)
#define DEFINE_MQO_DEF_SCENE_FRONTCLIP     (55)
#define DEFINE_MQO_DEF_SCENE_BACKCLIP      (11000)
#define DEFINE_MQO_DEF_SCENE_LIGHT_DIR_X   (0.4f)
#define DEFINE_MQO_DEF_SCENE_LIGHT_DIR_Y   (0.4f)
#define DEFINE_MQO_DEF_SCENE_LIGHT_DIR_Z   (0.8f)
#define DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_R (1.0f)
#define DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_G (1.0f)
#define DEFINE_MQO_DEF_SCENE_LIGHT_COLOR_B (1.0f)

#define DEFINE_MQO_DEF_OBJECT_DEPTH         (0)
#define DEFINE_MQO_DEF_OBJECT_FOLDING       (0)
#define DEFINE_MQO_DEF_OBJECT_SCALE_X       (1)
#define DEFINE_MQO_DEF_OBJECT_SCALE_Y       (1)
#define DEFINE_MQO_DEF_OBJECT_SCALE_Z       (1)
#define DEFINE_MQO_DEF_OBJECT_ROTATION_X    (0)
#define DEFINE_MQO_DEF_OBJECT_ROTATION_Y    (0)
#define DEFINE_MQO_DEF_OBJECT_ROTATION_Z    (0)
#define DEFINE_MQO_DEF_OBJECT_TRANSLATION_X (0)
#define DEFINE_MQO_DEF_OBJECT_TRANSLATION_Y (0)
#define DEFINE_MQO_DEF_OBJECT_TRANSLATION_Z (0)
#define DEFINE_MQO_DEF_OBJECT_VISIBLE       (15)
#define DEFINE_MQO_DEF_OBJECT_LOCKING       (1)
#define DEFINE_MQO_DEF_OBJECT_SHADING       (1)
#define DEFINE_MQO_DEF_OBJECT_FACET         (60.0f)
#define DEFINE_MQO_DEF_OBJECT_NORMALWEIGHT  (1)
#define DEFINE_MQO_DEF_OBJECT_COLOR_R       (0.900f)
#define DEFINE_MQO_DEF_OBJECT_COLOR_G       (0.500f)
#define DEFINE_MQO_DEF_OBJECT_COLOR_B       (0.700f)
#define DEFINE_MQO_DEF_OBJECT_COLORTYPE     (0)
#define DEFINE_MQO_DEF_OBJECT_NAME          ("no name")

// MQOオブジェクト構造体
struct type_mqo_object {
	//int    depth = DEFINE_MQO_DEF_OBJECT_DEPTH;
	//int    folding = DEFINE_MQO_DEF_OBJECT_FOLDING;
	//int    scale[3] = { DEFINE_MQO_DEF_OBJECT_SCALE_X,
	//						  DEFINE_MQO_DEF_OBJECT_SCALE_Y,
	//						  DEFINE_MQO_DEF_OBJECT_SCALE_Z };
	//int    rotation[3] = { DEFINE_MQO_DEF_OBJECT_ROTATION_X,
	//						  DEFINE_MQO_DEF_OBJECT_ROTATION_Y,
	//						  DEFINE_MQO_DEF_OBJECT_ROTATION_Z };
	//int    translation[3] = { DEFINE_MQO_DEF_OBJECT_TRANSLATION_X,
	//						  DEFINE_MQO_DEF_OBJECT_TRANSLATION_Y,
	//						  DEFINE_MQO_DEF_OBJECT_TRANSLATION_Z };
	//int    visible = DEFINE_MQO_DEF_OBJECT_VISIBLE;
	//int    locking = DEFINE_MQO_DEF_OBJECT_LOCKING;
	//int    shading = DEFINE_MQO_DEF_OBJECT_SHADING;
	//double facet = DEFINE_MQO_DEF_OBJECT_FACET;
	//int    normal_weight = DEFINE_MQO_DEF_OBJECT_NORMALWEIGHT;
	//double color[3] = { DEFINE_MQO_DEF_OBJECT_COLOR_R,
	//						  DEFINE_MQO_DEF_OBJECT_COLOR_G,
	//						  DEFINE_MQO_DEF_OBJECT_COLOR_B };
	//int    color_type = DEFINE_MQO_DEF_OBJECT_COLORTYPE;

	//string                  name = DEFINE_MQO_DEF_OBJECT_NAME;
	//vector<type_mqo_vertex> vertex;
	//vector<type_mqo_face>   face;

	int                     error;
};


#endif //__MQO_H__