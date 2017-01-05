/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// draw.c

#include "gl_local.h"

image_t		*draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload (void);


/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
	// load console characters (don't bilerp characters)
	draw_chars = GL_FindImage ("pics/conchars.pcx", "pics/conchars.pcx", it_pic);
	if (!draw_chars)
		VID_Error (ERR_FATAL, "Couldn't load conchars.pcx\n\nEither you aren't running Quake 2 from the correct directory or you are missing important files.");
	GL_MBind(GL_TEXTURE0, draw_chars->texnum);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static const float conchars_texoffset[16] =
{
	0, 0.0625, 0.125, 0.1875, 0.25, 0.3125, 0.375, 0.4375, 0.5, 0.5625, 0.625, 0.6875, 0.75, 0.8125, 0.875, 0.9375
};

static const float conchars_texlimits[16] =
{
	0.0625, 0.125, 0.1875, 0.25, 0.3125, 0.375, 0.4375, 0.5, 0.5625, 0.625, 0.6875, 0.75, 0.8125, 0.875, 0.9375, 1
};


void R_DrawString(int x, int y, const char *s, int xorVal, unsigned int len) {
	GL_MBind(GL_TEXTURE0, draw_chars->texnum);

	if (draw_chars->has_alpha)
	{
		qglDisable(GL_ALPHA_TEST);

		qglEnable(GL_BLEND);

		GL_TexEnv(GL_TEXTURE0, GL_MODULATE);
	}

	qglBegin (GL_TRIANGLES);

	for (unsigned int i = 0; i < len; i++)
	{
		int num = (s[i] ^ xorVal);

		num &= 0xFF;

		if ( (num&127) == 32 ) {
			x+=8;
			continue;		// space
		}

		//if (y <= -8)
		//	return;			// totally off screen

		int row = num>>4;
		int col = num&15;

		float frow = conchars_texoffset[row];
		float fcol = conchars_texoffset[col];

		float frowbottom = conchars_texlimits[row];
		float fcolbottom = conchars_texlimits[col];

		qglMTexCoord2f (GL_TEXTURE0, fcol, frow);
		qglVertex2f(x, y);

		qglMTexCoord2f (GL_TEXTURE0, fcolbottom, frow);
		qglVertex2f(x + 8, y);

		qglMTexCoord2f (GL_TEXTURE0, fcolbottom, frowbottom);
		qglVertex2f(x + 8, y + 8);

		qglMTexCoord2f (GL_TEXTURE0, fcolbottom, frowbottom);
		qglVertex2f(x + 8, y + 8);

		qglMTexCoord2f (GL_TEXTURE0, fcol, frow);
		qglVertex2f(x, y);

		qglMTexCoord2f (GL_TEXTURE0, fcol, frowbottom);
		qglVertex2f(x, y + 8);

		x+=8;
	}

	qglEnd ();

	if (draw_chars->has_alpha)
	{
		GL_TexEnv(GL_TEXTURE0, GL_REPLACE);
		
		qglEnable(GL_ALPHA_TEST);

		qglDisable(GL_BLEND);
	}
}


/*
================
R_DrawChar

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void R_DrawChar (int x, int y, int num)
{
	num &= 0xFF;

	if ( (num&127) == 32 )
		return;		// space

	//if (y <= -8)
	//	return;			// totally off screen

	int row = num>>4;
	int col = num&15;

	float frow = conchars_texoffset[row];
	float fcol = conchars_texoffset[col];

	float frowbottom = conchars_texlimits[row];
	float fcolbottom = conchars_texlimits[col];

	GL_MBind(GL_TEXTURE0, draw_chars->texnum);

	if (draw_chars->has_alpha)
	{
		qglDisable(GL_ALPHA_TEST);

		qglEnable(GL_BLEND);

		GL_TexEnv(GL_TEXTURE0, GL_MODULATE);
	}

	qglBegin (GL_TRIANGLES);

	qglMTexCoord2f (GL_TEXTURE0, fcol, frow);
	qglVertex2f(x, y);

	qglMTexCoord2f (GL_TEXTURE0, fcolbottom, frow);
	qglVertex2f(x + 8, y);

	qglMTexCoord2f (GL_TEXTURE0, fcolbottom, frowbottom);
	qglVertex2f(x + 8, y + 8);

	qglMTexCoord2f (GL_TEXTURE0, fcolbottom, frowbottom);
	qglVertex2f(x + 8, y + 8);

	qglMTexCoord2f (GL_TEXTURE0, fcol, frow);
	qglVertex2f(x, y);

	qglMTexCoord2f (GL_TEXTURE0, fcol, frowbottom);
	qglVertex2f(x, y + 8);

	qglEnd ();

	if (draw_chars->has_alpha)
	{
		GL_TexEnv(GL_TEXTURE0, GL_REPLACE);
		
		qglEnable(GL_ALPHA_TEST);

		qglDisable(GL_BLEND);
	}
}


/*
=============
Draw_FindPic
=============
*/
image_t	* Draw_FindPic (char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	fast_strlwr (name);

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage (fullname, name, it_pic);
	}
	else
		gl = GL_FindImage (name+1, name+1, it_pic);

	return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		*w = *h = -1;
		return;
	}

	*w = gl->width;
	*h = gl->height;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic (int x, int y, int w, int h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		Com_DPrintf("Can't find pic: %s\n", pic);
		gl = r_notexture;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	if (gl->has_alpha)
	{
		qglDisable(GL_ALPHA_TEST);

		qglEnable(GL_BLEND);

		GL_TexEnv(GL_TEXTURE0, GL_MODULATE);
	}

	GL_MBind(GL_TEXTURE0, gl->texnum);
	qglBegin (GL_TRIANGLES);

	qglMTexCoord2f(GL_TEXTURE0, gl->sl, gl->tl);
	qglVertex2f(x, y);

	qglMTexCoord2f(GL_TEXTURE0, gl->sh, gl->tl);
	qglVertex2f(x + w, y);

	qglMTexCoord2f(GL_TEXTURE0, gl->sh, gl->th);
	qglVertex2f(x + w, y + h);

	qglMTexCoord2f(GL_TEXTURE0, gl->sh, gl->th);
	qglVertex2f(x + w, y + h);

	qglMTexCoord2f(GL_TEXTURE0, gl->sl, gl->tl);
	qglVertex2f(x, y);

	qglMTexCoord2f(GL_TEXTURE0, gl->sl, gl->th);
	qglVertex2f(x, y + h);

	qglEnd ();

	if (gl->has_alpha)
	{
		GL_TexEnv(GL_TEXTURE0, GL_REPLACE);
		
		qglEnable(GL_ALPHA_TEST);

		qglDisable(GL_BLEND);
	}
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);

	if (!gl)
	{
		Com_DPrintf("Can't find pic: %s\n", pic);
		gl = r_notexture;
	}

	if (scrap_dirty)
		Scrap_Upload ();

	if (gl->has_alpha)
	{
		qglDisable(GL_ALPHA_TEST);

		qglEnable(GL_BLEND);

		GL_TexEnv(GL_TEXTURE0, GL_MODULATE);
	}

	GL_MBind(GL_TEXTURE0, gl->texnum);

	qglBegin(GL_TRIANGLES);

	qglMTexCoord2f(GL_TEXTURE0, gl->sl, gl->tl);
	qglVertex2f(x, y);

	qglMTexCoord2f(GL_TEXTURE0, gl->sh, gl->tl);
	qglVertex2f(x + gl->width, y);

	qglMTexCoord2f(GL_TEXTURE0, gl->sh, gl->th);
	qglVertex2f(x + gl->width, y+gl->height);

	qglMTexCoord2f(GL_TEXTURE0, gl->sh, gl->th);
	qglVertex2f(x + gl->width, y+gl->height);

	qglMTexCoord2f(GL_TEXTURE0, gl->sl, gl->tl);
	qglVertex2f(x, y);

	qglMTexCoord2f(GL_TEXTURE0, gl->sl, gl->th);
	qglVertex2f(x, y + gl->height);

	qglEnd();

	if (gl->has_alpha)
	{
		GL_TexEnv(GL_TEXTURE0, GL_REPLACE);
		qglEnable(GL_ALPHA_TEST);
		qglDisable(GL_BLEND);
	}

	if ( ( ( false) || ( false ) )  && !gl->has_alpha)
	{
		qglEnable (GL_ALPHA_TEST);
	}
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h, char *pic)
{
	image_t	*image;

	image = Draw_FindPic (pic);

	if (!image)
	{
		Com_DPrintf("Can't find pic: %s\n", pic);
		image = r_notexture;
	}

	GL_MBind(GL_TEXTURE0, image->texnum);
	qglBegin (GL_QUADS);
	qglMTexCoord2f(GL_TEXTURE0, x/64.0f, y/64.0f);
	qglVertex2f(x, y);
	qglMTexCoord2f(GL_TEXTURE0,  (x+w)/64.0f, y/64.0f);
	qglVertex2f(x+w, y);
	qglMTexCoord2f(GL_TEXTURE0,  (x+w)/64.0f, (y+h)/64.0f);
	qglVertex2f(x+w, y+h);
	qglMTexCoord2f(GL_TEXTURE0,  x/64.0f, (y+h)/64.0f );
	qglVertex2f(x, y+h);
	qglEnd ();
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	union
	{
		unsigned	c;
		byte		v[4];
	} color;

	if ( (unsigned)c > 255)
		VID_Error (ERR_FATAL, "Draw_Fill: bad color");

	qglDisable (GL_TEXTURE_2D);

	color.c = d_8to24table[c];
	qglColor3f (color.v[0]/255.0f,
		color.v[1]/255.0f,
		color.v[2]/255.0f);

	qglBegin(GL_TRIANGLES);

	qglVertex2f(x, y);

	qglVertex2f(x + w, y);

	qglVertex2f(x + w, y + h);

	qglVertex2f(x + w, y + h);

	qglVertex2f(x, y);

	qglVertex2f(x, y + h);

	qglEnd();

	qglColor3f (1,1,1);

	qglEnable (GL_TEXTURE_2D);
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
	qglEnable (GL_BLEND);

	qglDisable (GL_TEXTURE_2D);

	qglColor4f (0, 0, 0, 0.8f);

	qglBegin (GL_TRIANGLES);

	qglVertex2f(0,0);
	qglVertex2f(viddef.width, 0);
	qglVertex2f(viddef.width, viddef.height);

	qglVertex2f(viddef.width, viddef.height);
	qglVertex2f(0,0);
	qglVertex2f(0, viddef.height);

	qglEnd ();

	qglColor4f(colorWhite[0], colorWhite[1], colorWhite[2], colorWhite[3]);

	qglEnable (GL_TEXTURE_2D);

	qglDisable (GL_BLEND);
}
