void R_Init() {	
#if 0
	int	err;
	int i;
	byte *ptr;

	ri.Printf( PRINT_ALL, "----- R_Init -----\n" );

	// clear all our internal state
	memset( &tr, 0, sizeof( tr ) );
	memset( &backEnd, 0, sizeof( backEnd ) );
	memset( &tess, 0, sizeof( tess ) );

//	Swap_Init();

	if ( (int)tess.xyz & 15 ) {
		Com_Printf( "WARNING: tess.xyz not 16 byte aligned\n" );
	}
	memset( tess.constantColor255, 255, sizeof( tess.constantColor255 ) );
#endif

	//
	// init function tables
	//
	//for ( i = 0; i < FUNCTABLE_SIZE; i++ )
	//{
	//	tr.sinTable[i]		= sin( DEG2RAD( i * 360.0f / ( ( float ) ( FUNCTABLE_SIZE - 1 ) ) ) );
	//	tr.squareTable[i]	= ( i < FUNCTABLE_SIZE/2 ) ? 1.0f : -1.0f;
	//	tr.sawToothTable[i] = (float)i / FUNCTABLE_SIZE;
	//	tr.inverseSawToothTable[i] = 1.0f - tr.sawToothTable[i];

	//	if ( i < FUNCTABLE_SIZE / 2 )
	//	{
	//		if ( i < FUNCTABLE_SIZE / 4 )
	//		{
	//			tr.triangleTable[i] = ( float ) i / ( FUNCTABLE_SIZE / 4 );
	//		}
	//		else
	//		{
	//			tr.triangleTable[i] = 1.0f - tr.triangleTable[i-FUNCTABLE_SIZE / 4];
	//		}
	//	}
	//	else
	//	{
	//		tr.triangleTable[i] = -tr.triangleTable[i-FUNCTABLE_SIZE/2];
	//	}
	//}

	//R_InitFogTable();

	//R_NoiseInit();

	//R_Register();

#if 0
	max_polys = r_maxpolys->integer;
	if (max_polys < MAX_POLYS)
		max_polys = MAX_POLYS;

	max_polyverts = r_maxpolyverts->integer;
	if (max_polyverts < MAX_POLYVERTS)
		max_polyverts = MAX_POLYVERTS;

	ptr = ri.Hunk_Alloc( sizeof( *backEndData[0] ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, h_low);
	backEndData[0] = (backEndData_t *) ptr;
	backEndData[0]->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData[0] ));
	backEndData[0]->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData[0] ) + sizeof(srfPoly_t) * max_polys);
#endif

	//if ( r_smp->integer ) {
	//	ptr = ri.Hunk_Alloc( sizeof( *backEndData[1] ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts, h_low);
	//	backEndData[1] = (backEndData_t *) ptr;
	//	backEndData[1]->polys = (srfPoly_t *) ((char *) ptr + sizeof( *backEndData[1] ));
	//	backEndData[1]->polyVerts = (polyVert_t *) ((char *) ptr + sizeof( *backEndData[1] ) + sizeof(srfPoly_t) * max_polys);
	//} else {
	//	backEndData[1] = NULL;
	//}
	//R_ToggleSmpFrame();

	//InitOpenGL();

	//R_InitImages();

	//R_InitShaders();

	//R_InitSkins();

	//R_ModelInit();

	//R_InitFreeType();


	//err = qglGetError();
	//if ( err != GL_NO_ERROR )
	//	ri.Printf (PRINT_ALL, "glGetError() = 0x%x\n", err);

	//ri.Printf( PRINT_ALL, "----- finished R_Init -----\n" );
}
