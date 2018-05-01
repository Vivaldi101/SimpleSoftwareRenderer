#include "shared.h"
#include "renderer.h"
#include "lights.h"

// flat shading
void R_CalculateLighting(const RendererBackend *rb, AmbientState as, Vec3 camera_dir, Vec3 camera_pos) {
	Vec4 base = {};
	Vec4 total = {};
	int num_polys = rb->num_polys;
	int num_lights = rb->num_lights;
   Light *lights = rb->lights;
	for (int i = 0; i < num_polys; ++i) {
      Vec4 color = {};
		if ((rb->polys[i].state & POLY_STATE_LIT)) {
			continue;
		}
		rb->polys[i].state |= POLY_STATE_LIT;

		base = rb->polys[i].color;

		PolyVert v0 = rb->polys[i].vertex_array[0];
		PolyVert v1 = rb->polys[i].vertex_array[1];
		PolyVert v2 = rb->polys[i].vertex_array[2];

		for (int j = 0; j < num_lights; ++j) {
			if (!lights[j].is_active) {
				continue;
			}
			Vec4 ambient = {};
			Vec4 diffuse = {};
			Vec4 specular = {};

			// ambient component
			ambient.c.a = base.c.a * (lights[j].ambient.c.a * as);		
			ambient.c.r = base.c.r * (lights[j].ambient.c.r * as);		
			ambient.c.g = base.c.g * (lights[j].ambient.c.g * as);		
			ambient.c.b = base.c.b * (lights[j].ambient.c.b * as);		

			Vec3 u = MakeVec3(v0.xyz, v1.xyz);
			Vec3 v = MakeVec3(v0.xyz, v2.xyz);
			Vec3 n = -Cross3(u, v);		// NOTE: negated because of left-handed system and because we are using ccw winding order

			Vec3 l = (lights[j].flags & CAMERA_LIGHT) ? MakeVec3(v0.xyz, (camera_pos)) : MakeVec3(v0.xyz, lights[j].pos); 
			r32 llen = Vec3Len(l);
			l = Vec3Norm(l);

			r32 atten = (lights[j].flags & POINT_LIGHT || lights[j].flags & SPOT_LIGHT) ? (1.0f / (lights[j].kc + (lights[j].kl * ABS(llen)) + (lights[j].kq * Square(ABS(llen))))) : 1;
			r32 factor = Dot3(Vec3Norm(camera_dir), (-l));
			r32 spot = (lights[j].flags & SPOT_LIGHT) ? (pow(MAX(factor, 0.0f), 32)) : 1.0f;

			r32 nlen = Vec3Len(n);
			n = Vec3Norm(n);

			Vec3 view = MakeVec3(v0.xyz, camera_pos); 
			view = Vec3Norm(view);


			// diffuse component
			r32 dot = MAX(Dot3(n, l), 0.0f);
			diffuse.c.a = base.c.a * lights[j].diffuse.c.a;
			diffuse.c.r = base.c.r * lights[j].diffuse.c.r;
			diffuse.c.g = base.c.g * lights[j].diffuse.c.g;
			diffuse.c.b = base.c.b * lights[j].diffuse.c.b;
			diffuse = diffuse * dot;

			// specular component
			Vec3 h = l + view;
			dot = pow(MAX(Dot3(n, h), 0.0f), 3);
			specular.c.a = base.c.a * lights[j].specular.c.a;
			specular.c.r = base.c.r * lights[j].specular.c.r;
			specular.c.g = base.c.g * lights[j].specular.c.g;
			specular.c.b = base.c.b * lights[j].specular.c.b;
			specular = specular * dot;

			total = spot * (ambient + (atten * (diffuse + specular)));

			total.c.a = MIN(total.c.a, 1.0f);
			total.c.r = MIN(total.c.r, 1.0f);
			total.c.g = MIN(total.c.g, 1.0f);
			total.c.b = MIN(total.c.b, 1.0f);

			color = color + total;

			color.c.a = MIN(color.c.a, 1.0f);
			color.c.r = MIN(color.c.r, 1.0f);
			color.c.g = MIN(color.c.g, 1.0f);
			color.c.b = MIN(color.c.b, 1.0f);
		}

		rb->polys[i].color = color;
	}
}

void R_AddLight(RendererBackend *rb, Vec4 ambient, Vec4 diffuse, Vec4 specular, Vec3 pos, r32 radius, r32 kc, r32 kl, r32 kq, LightTypeFlags sf) {
	Assert(rb->num_lights + 1 <= MAX_NUM_LIGHTS);
	int i = rb->num_lights;
	rb->lights[i].ambient = ambient;
	rb->lights[i].diffuse = diffuse;
	rb->lights[i].specular = specular;
	rb->lights[i].pos = pos;
	rb->lights[i].kc = kc;
	rb->lights[i].kl = kl;
	rb->lights[i].kq = kq;
	rb->lights[i].is_active = true;
	rb->lights[i].radius = radius;
	rb->lights[i].flags |= sf;
	rb->num_lights++;
}

void R_AddLight(RendererBackend *rb, const Light *l) {
	//Assert(rb->num_lights + 1 <= MAX_NUM_LIGHTS);
	int i = rb->num_lights;
	memcpy(&rb->lights[i], l, sizeof(*l));
	//rb->lights[i] = *l;
	rb->num_lights++;
}
