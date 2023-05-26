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
#define DEFINE_MQO_DEF_OBJECT_LOCKING       (0)
#define DEFINE_MQO_DEF_OBJECT_SHADING       (1)
#define DEFINE_MQO_DEF_OBJECT_FACET         (60.0f)
#define DEFINE_MQO_DEF_OBJECT_NORMALWEIGHT  (1)
#define DEFINE_MQO_DEF_OBJECT_COLOR_R       (0.900f)
#define DEFINE_MQO_DEF_OBJECT_COLOR_G       (0.500f)
#define DEFINE_MQO_DEF_OBJECT_COLOR_B       (0.700f)
#define DEFINE_MQO_DEF_OBJECT_COLORTYPE     (0)
#define DEFINE_MQO_DEF_OBJECT_NAME          ("no name")

class MQO {
public:
	/* 頂点構造体 */
	struct type_vertex {
		double x;
		double y;
		double z;
		uint32 id;
	};

	/* 平面構造体 */
	struct type_face {
		vector<uint32> vertex;
		int32          material;
		uint32         id;
	};

	/* MQOオブジェクト構造体 */
	struct type_object {
		int depth = DEFINE_MQO_DEF_OBJECT_DEPTH;
		int folding = DEFINE_MQO_DEF_OBJECT_FOLDING;
		int scale[3] = {
			DEFINE_MQO_DEF_OBJECT_SCALE_X,
			DEFINE_MQO_DEF_OBJECT_SCALE_Y,
			DEFINE_MQO_DEF_OBJECT_SCALE_Z
		};
		int rotation[3] = {
			DEFINE_MQO_DEF_OBJECT_ROTATION_X,
			DEFINE_MQO_DEF_OBJECT_ROTATION_Y,
			DEFINE_MQO_DEF_OBJECT_ROTATION_Z
		};
		int translation[3] = {
			DEFINE_MQO_DEF_OBJECT_TRANSLATION_X,
			DEFINE_MQO_DEF_OBJECT_TRANSLATION_Y,
			DEFINE_MQO_DEF_OBJECT_TRANSLATION_Z
		};
		int visible = DEFINE_MQO_DEF_OBJECT_VISIBLE;
		int locking = DEFINE_MQO_DEF_OBJECT_LOCKING;
		int shading = DEFINE_MQO_DEF_OBJECT_SHADING;
		double facet = DEFINE_MQO_DEF_OBJECT_FACET;
		int    normal_weight = DEFINE_MQO_DEF_OBJECT_NORMALWEIGHT;
		double color[3] = {
			DEFINE_MQO_DEF_OBJECT_COLOR_R,
			DEFINE_MQO_DEF_OBJECT_COLOR_G,
			DEFINE_MQO_DEF_OBJECT_COLOR_B
		};
		int                 color_type = DEFINE_MQO_DEF_OBJECT_COLORTYPE;
		string              name = DEFINE_MQO_DEF_OBJECT_NAME;
		vector<type_vertex> vertex;
		vector<type_face>   face;
	};

private:
	/* ヘッダ構造体 */
	struct type_header {
		string format = DEFINE_MQO_DEF_HEADER_FORMAT;
		float  version = DEFINE_MQO_DEF_HEADER_VERSION;
		string codepage = DEFINE_MQO_DEF_HEADER_CODEPAGE;
	};

	/* 方向構造体 */
	struct type_direction {
		float x;
		float y;
		float z;
	};

	/* 色構造体 */
	struct type_color {
		float r;
		float g;
		float b;
	};

	/* 照明構造体 */
	struct type_light {
		string         name;
		type_direction direction;
		type_color     color;
	};

	/* シーン構造体 */
	struct type_scene {
		float pos[3] = {
			DEFINE_MQO_DEF_SCENE_POS_X,
			DEFINE_MQO_DEF_SCENE_POS_Y,
			DEFINE_MQO_DEF_SCENE_POS_Z
		};
		int32 lookat[3] = {
			DEFINE_MQO_DEF_SCENE_LOOKAT_X,
			DEFINE_MQO_DEF_SCENE_LOOKAT_Y,
			DEFINE_MQO_DEF_SCENE_LOOKAT_Z
		};
		float head = DEFINE_MQO_DEF_SCENE_HEAD;
		float pich = DEFINE_MQO_DEF_SCENE_PICH;
		float bank = DEFINE_MQO_DEF_SCENE_BANK;
		int32 ortho = DEFINE_MQO_DEF_SCENE_ORTHO;
		float zoom2 = DEFINE_MQO_DEF_SCENE_ZOOM2;
		float amb[3] = {
			DEFINE_MQO_DEF_SCENE_AMB_X,
			DEFINE_MQO_DEF_SCENE_AMB_Y,
			DEFINE_MQO_DEF_SCENE_AMB_Z
		};
		double frontclip = DEFINE_MQO_DEF_SCENE_FRONTCLIP;
		double backclip = DEFINE_MQO_DEF_SCENE_BACKCLIP;
		vector<type_light> dirlights;
	};

public:
	type_object m_object;

private:
	type_header m_header;
	type_scene  m_scene;

public:
	MQO();

public:
	void Write(const string file_path, const string marge_from);
	void WriteStl(const string file_path, const string marge_from);

private:
	void writeHeader(ofstream* p_fout);
	void writeScene(ofstream* p_fout);
	void writeObject(ofstream* p_fout);
};

#endif //__MQO_H__
