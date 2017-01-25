#include "shared.h"
#include "renderer_local.h"

void R_BeginFrame() {
	// clean up the framebuffer
	void *buffer = global_renderer->vid_sys.buffer;
	int pitch = global_renderer->vid_sys.pitch;
	int height = global_renderer->vid_sys.height;
	memset(buffer, 0, pitch * height);
}

void R_EndFrame() {
	if (!global_renderer->vid_sys.state.full_screen) {
	//	
		//printf(" sww_state.pDIBBase ok (%u).\n", sww_state.pDIBBase);

		//memset(vid.buffer          ,200  , vid.width * vid.height );
		//printf(" vid.buffer         ok (%u).\n", vid.buffer);
/*
		printf("SWimp_EndFrame w=%d, h=%d, vid.rowbytes=%d\n",vid.width,vid.height,vid.rowbytes);
		//memset(vid.buffer                 , 200, vid.width * 2 );
		//memset(vid.buffer + 100*vid.rowbytes, 200, vid.width *150);
		
		{
			int i;
			//int acc=0;
				for (i=0 ; i<vid.height ; i++)
				{
					memset (vid.buffer + i*vid.rowbytes, 200, vid.width);
					//acc+= vid.width*2 ;
					
				}
				//printf("cleared %d bytes.\n",acc);
				//printf("diff acc Vs vid.width * vid.height = %d\n",acc - vid.width * vid.height );
		}
*/		

		BitBlt( global_renderer->vid_sys.win_handles.dc,
				0, 0,
				global_renderer->vid_sys.width,
				global_renderer->vid_sys.height,
				global_renderer->vid_sys.win_handles.hdc_dib_section,
				0, 0, SRCCOPY );
	}
#if 0
	else {
		RECT r;
		HRESULT rval;
		DDSURFACEDESC ddsd;

		r.left = 0;
		r.top = 0;
		r.right = vid.width;
		r.bottom = vid.height;

		
		sww_state.lpddsOffScreenBuffer->lpVtbl->Unlock( sww_state.lpddsOffScreenBuffer, vid.buffer );

		if ( sww_state.modex )
		{
			if ( ( rval = sww_state.lpddsBackBuffer->lpVtbl->BltFast( sww_state.lpddsBackBuffer,
																	0, 0,
																	sww_state.lpddsOffScreenBuffer, 
																	&r, 
																	DDBLTFAST_WAIT ) ) == DDERR_SURFACELOST )
			{
				sww_state.lpddsBackBuffer->lpVtbl->Restore( sww_state.lpddsBackBuffer );
				sww_state.lpddsBackBuffer->lpVtbl->BltFast( sww_state.lpddsBackBuffer,
															0, 0,
															sww_state.lpddsOffScreenBuffer, 
															&r, 
															DDBLTFAST_WAIT );
			}

			if ( ( rval = sww_state.lpddsFrontBuffer->lpVtbl->Flip( sww_state.lpddsFrontBuffer,
															 NULL, DDFLIP_WAIT ) ) == DDERR_SURFACELOST )
			{
				sww_state.lpddsFrontBuffer->lpVtbl->Restore( sww_state.lpddsFrontBuffer );
				sww_state.lpddsFrontBuffer->lpVtbl->Flip( sww_state.lpddsFrontBuffer, NULL, DDFLIP_WAIT );
			}
		}
		else
		{
			if ( ( rval = sww_state.lpddsBackBuffer->lpVtbl->BltFast( sww_state.lpddsFrontBuffer,
																	0, 0,
																	sww_state.lpddsOffScreenBuffer, 
																	&r, 
																	DDBLTFAST_WAIT ) ) == DDERR_SURFACELOST )
			{
				sww_state.lpddsBackBuffer->lpVtbl->Restore( sww_state.lpddsFrontBuffer );
				sww_state.lpddsBackBuffer->lpVtbl->BltFast( sww_state.lpddsFrontBuffer,
															0, 0,
															sww_state.lpddsOffScreenBuffer, 
															&r, 
															DDBLTFAST_WAIT );
			}
		}

		memset( &ddsd, 0, sizeof( ddsd ) );
		ddsd.dwSize = sizeof( ddsd );
	
		sww_state.lpddsOffScreenBuffer->lpVtbl->Lock( sww_state.lpddsOffScreenBuffer, NULL, &ddsd, DDLOCK_WAIT, NULL );

		vid.buffer = ddsd.lpSurface;
		
		vid.rowbytes = ddsd.lPitch;

		//memset(vid.buffer,200,1024*768);
	}
#endif
}

#if 0
void R_FadeScreen() {
	int	t;

	int w = global_renderer.vid_sys.width;
	int h = global_renderer.vid_sys.height;
	int pitch = global_renderer.vid_sys.pitch;

	byte *buffer;
	for (int y = 0; y < h; y++) {
		buffer = global_renderer.vid_sys.buffer + (pitch * y);
		t = (y & 1) << 1;

		for (int x = 0; x < w; x++) {
			if ((x & 3) != t) {
				buffer[x] = 0;
			}
		}
	}
}
#endif

