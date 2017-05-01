#ifndef LIGHTS_H
#define LIGHTS_H

enum AmbientState {
	AMBIENT_OFF = 0,
	AMBIENT_ON = 1,
};

enum LightTypeFlags {
	POINT_LIGHT = 0x01 ,
	SPOTLIGHT1_LIGHT = 0x02,	// simple
	SPOTLIGHT2_LIGHT = 0x04		// complex
};

enum LightStatusFlags {
	CAMERA_LIGHT
};

struct Light {
	Vec4 ambient;	
	Vec4 diffuse;	
	Vec4 specular;	

	Vec3 pos; 
	Vec3 dir; 

	r32 kc, kl, kq;		// attenuation factors
	//r32 inner_cone;	// inner angle for spot light, D3D style spotlight
	//r32 outer_cone;	// outer angle for spot light
	r32 pf;				// power factor/falloff for positional lights

	r32	radius;
	u32	is_active;
	u32	light_flags;
	u32 status_flags;
};

struct RendererBackend;
struct ViewSystem;
extern void R_CalculateLighting(const RendererBackend *rb, const Light *lights, AmbientState as = AMBIENT_ON, Vec3 camera_pos = MakeVec3(0.0f, 0.0f, 0.0f));
extern void R_AddPointLight(RendererBackend *rb, Vec4 ambient, Vec4 diffuse, Vec4 specular, Vec3 pos, r32 radius, r32 kc, r32 kl, r32 kq, LightStatusFlags sf);
#endif	// header guard