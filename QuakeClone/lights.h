#ifndef LIGHTS_H
#define LIGHTS_H

enum AmbientState {
	AMBIENT_OFF = 0,
	AMBIENT_ON = 1,
};

enum LightTypeFlags {
	POINT_LIGHT = 0x01,
	SPOT_LIGHT = 0x02,	
	CAMERA_LIGHT = 0x04
};

struct Light {
	Vec4 ambient;	
	Vec4 diffuse;	
	Vec4 specular;	

	Vec3 pos; 
	r32	 radius;
	//Vec3 dir;			// no directional lights atm 

	r32 kc, kl, kq;		// attenuation factors
	//r32 inner_cone;	// inner angle for spot light, D3D style spotlight
	//r32 outer_cone;	// outer angle for spot light
	r32 pf;				// power factor/falloff for positional lights

	u32	is_active;
	u32	flags;
};

struct RendererBackend;
struct ViewSystem;
extern void R_CalculateLighting(const RendererBackend *rb, const Light *lights, AmbientState as, Vec3 camera_dir, Vec3 camera_pos);
extern void R_AddLight(RendererBackend *rb, Vec4 ambient, Vec4 diffuse, Vec4 specular, Vec3 pos, r32 radius, r32 kc, r32 kl, r32 kq, LightTypeFlags sf);
extern void R_AddLight(RendererBackend *rb, const Light *l);
#endif	// header guard