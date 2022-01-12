ray_t r = {
		.start = current_point,
		.direction = ray_dir
	};

	collision_t t = collision_ray_sphere(&r, &sphere_at_zero); 

	p->position = temp_p;

	if(t.collided && t.t <= 1) {

		sh_vec3 norm = t.intersection_point;
		sh_vec3_sub_vec3(&norm, &sphere_at_zero.center);
		sh_vec3_normalize_ref(&norm);

		p->position.x = t.intersection_point.x;
		p->position.y = t.intersection_point.y;
		p->position.z = t.intersection_point.z;

		f32 amount = sh_vec3_dot(&p->velocity, &norm);

		if(amount < 0) amount = 0;

		sh_vec3_mul_scaler(&norm, amount);
		sh_vec3_add_vec3(&p->velocity, &norm);

	} 

	sh_vec3_mul_scaler(&p->velocity, 0.98f);

	p->total_force.x = 0;
	p->total_force.y = 0;
	p->total_force.z = 0;

