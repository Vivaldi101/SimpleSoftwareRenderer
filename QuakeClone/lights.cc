#include "shared.h"
#include "renderer.h"
#include "lights.h"

// some simple flat shading
// no specular lighting component and 
// also no directional light sources atm
void R_CalculateLighting(const RendererBackend *rb, const Light *lights, AmbientState as, Vec3 camera_pos) {
	Vec4 base = {};
	Vec4 total = {};

	int num_polys = rb->num_polys;
	int num_lights = rb->num_lights;
	for (int i = 0; i < num_polys; ++i) {
		u32 color = 0;
		if (!(rb->polys[i].state & POLY_STATE_ACTIVE)) {
			continue;
		}

		// FIXME: make the bit ops nicer
		base.c.a = (r32)(((rb->polys[i].color & 0xff000000) >> 24 ) / 255.0f);
		base.c.r = (r32)(((rb->polys[i].color & 0xff0000) >> 16 ) / 255.0f);
		base.c.g = (r32)(((rb->polys[i].color & 0xff00) >> 8) / 255.0f);
		base.c.b = (r32)((rb->polys[i].color & 0xff) / 255.0f);

		Vec3 v0 = rb->polys[i].vertex_array[0];
		Vec3 v1 = rb->polys[i].vertex_array[1];
		Vec3 v2 = rb->polys[i].vertex_array[2];

		for (int j = 0; j < num_lights; ++j) {
			if (!lights[j].is_active) {
				continue;
			}
			Vec4 ambient = {};
			Vec4 diffuse = {};
			Vec4 specular = {};
			ambient.c.a = base.c.a * (lights[j].ambient.c.a * as);		
			ambient.c.r = base.c.r * (lights[j].ambient.c.r * as);		
			ambient.c.g = base.c.g * (lights[j].ambient.c.g * as);		
			ambient.c.b = base.c.b * (lights[j].ambient.c.b * as);		

			if (lights[j].light_flags & POINT_LIGHT) {
				Vec3 u = MakeVec3(v0, v1);
				Vec3 v = MakeVec3(v0, v2);
				Vec3 n = Vec3Cross(u, v);

				r32 nlen = Vec3Len(n);
				n = Vec3Norm(n);

				Vec3 view = MakeVec3(v0, camera_pos); 
				view = Vec3Norm(view);

				Vec3 l = (lights[j].status_flags & CAMERA_LIGHT) ? MakeVec3(v0, camera_pos) : MakeVec3(v0, lights[j].pos); 
				r32 llen = Vec3Len(l);
				l = Vec3Norm(l);

				// diffuse component
				r32 dot = MAX(Vec3Dot(n, l), 0.0f);
				diffuse.c.a = base.c.a * lights[j].diffuse.c.a;
				diffuse.c.r = base.c.r * lights[j].diffuse.c.r;
				diffuse.c.g = base.c.g * lights[j].diffuse.c.g;
				diffuse.c.b = base.c.b * lights[j].diffuse.c.b;
				diffuse = diffuse * dot;

				// specular component
				Vec3 h = l + view;
				dot = MAX(pow(Vec3Dot(n, h), 4), 0.0f);
				specular.c.a = base.c.a * lights[j].specular.c.a;
				specular.c.r = base.c.r * lights[j].specular.c.r;
				specular.c.g = base.c.g * lights[j].specular.c.g;
				specular.c.b = base.c.b * lights[j].specular.c.b;
				specular = specular * dot;

				r32 atten = (1.0f / (lights[j].kc + (lights[j].kl * ABS(llen)) + (lights[j].kq * Square(ABS(llen)))));
				total = ambient + (atten * (diffuse + specular));

				total.c.a = MIN(total.c.a, 1.0f);
				total.c.r = MIN(total.c.r, 1.0f);
				total.c.g = MIN(total.c.g, 1.0f);
				total.c.b = MIN(total.c.b, 1.0f);

				color += (roundReal32ToU32(total.c.a * 255.0f) << 24 |
						 roundReal32ToU32(total.c.r * 255.0f) << 16 |
						 roundReal32ToU32(total.c.g * 255.0f) << 8  |
						 roundReal32ToU32(total.c.b * 255.0f));
			} else {
				color = rb->polys[i].color;
			}
		}

		rb->polys[i].color = color;
	}
}

// FIXME: no specular atm
void R_AddPointLight(RendererBackend *rb, Vec4 ambient, Vec4 diffuse, Vec4 specular, Vec3 pos, r32 radius, r32 kc, r32 kl, r32 kq, LightStatusFlags sf) {
	Assert(rb->num_lights + 1 <= MAX_NUM_LIGHTS);
	int light = rb->num_lights;
	rb->lights[light].ambient = ambient;
	rb->lights[light].diffuse = diffuse;
	rb->lights[light].specular = specular;
	rb->lights[light].pos = pos;
	rb->lights[light].kc = kc;
	rb->lights[light].kl = kl;
	rb->lights[light].kq = kq;
	rb->lights[light].is_active = true;
	rb->lights[light].radius = radius;
	rb->lights[light].light_flags |= POINT_LIGHT;
	rb->lights[light].status_flags |= sf;
	++rb->num_lights;
}
