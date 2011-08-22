
/**
 * @file psnr_ssim.h
 * psnr ssim quality metrics calculation
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __PSNR_SSIM_H__
#define __PSNR_SSIM_H__

#include <assert.h>
#include <math.h>

//////////////////////////////////////////////////////////////////////////
typedef struct psnr_ssim_t
{
//protected:
	int num_frames;
	int max_psnr;
	int ssim_enabled;
//public:
	// 3 elements: Y/U/V
	double ssim_frame[3];
	double psnr_frame[3];
	double ssim_total[3];
	double psnr_total[3];
	double mse_total[3];
//protected:
	short *img12_sum_block;
	short *img1_sum;
	short *img2_sum;
	int *img1_sq_sum;
	int *img2_sq_sum;
	int *img12_mul_sum;
} psnr_ssim_t;


static double similarity(int muX, int muY, int preMuX2, int preMuY2, int preMuXY2)
{
#define C1 (float)(64 * 64 * 0.01 * 255 * 0.01 * 255)
#define C2 (float)(64 * 64 * 0.03 * 255 * 0.03 * 255)
	int muX2, muY2, muXY, thetaX2, thetaY2, thetaXY;
	muX2 = muX*muX;
	muY2 = muY*muY;
	muXY = muX*muY;
	thetaX2 = 64 * preMuX2 - muX2;
	thetaY2 = 64 * preMuY2 - muY2;
	thetaXY = 64 * preMuXY2 - muXY;
	return (2 * muXY + C1) * (2 * thetaXY + C2) / ((muX2 + muY2 + C1) * (thetaX2 + thetaY2 + C2));
}


static double calc_ssim_plane(psnr_ssim_t *pss, byte *img1, int stride_img1, byte *img2, int stride_img2, int width, int height)
{
	int x, y, img1_block, img2_block, img1_sq_block, img2_sq_block, img12_mul_block, temp;
	double planeSummedWeights, planeQuality;
	short *img1_sum_ptr1, *img1_sum_ptr2;
	short *img2_sum_ptr1, *img2_sum_ptr2;
	int *img1_sq_sum_ptr1, *img1_sq_sum_ptr2;
	int *img2_sq_sum_ptr1, *img2_sq_sum_ptr2;
	int *img12_mul_sum_ptr1, *img12_mul_sum_ptr2;
	int stride = (width / 8 + 1) * 8;

	if (width*height <= 0) return 0.0;
	planeQuality = 0;
	planeSummedWeights = (height - 7) * (width - 7);
	temp = 8 * stride;
	img1_sum_ptr1      = pss->img1_sum + temp;
	img2_sum_ptr1      = pss->img2_sum + temp;
	img1_sq_sum_ptr1   = pss->img1_sq_sum + temp;
	img2_sq_sum_ptr1   = pss->img2_sq_sum + temp;
	img12_mul_sum_ptr1 = pss->img12_mul_sum + temp;
	
	for (x = 0; x < width; x++)
	{
		pss->img1_sum[x]      = img1[x];
		pss->img2_sum[x]      = img2[x];
		pss->img1_sq_sum[x]   = img1[x] * img1[x];
		pss->img2_sq_sum[x]   = img2[x] * img2[x];
		pss->img12_mul_sum[x] = img1[x] * img2[x];
		img1_sum_ptr1[x]      = 0;
		img2_sum_ptr1[x]      = 0;
		img1_sq_sum_ptr1[x]   = 0;
		img2_sq_sum_ptr1[x]   = 0;
		img12_mul_sum_ptr1[x] = 0;
	}
	
	//the main loop
	for (y = 1; y < height; y++)
	{
		img1 += stride_img1;
		img2 += stride_img2;
		temp = (y - 1) % 9 * stride;
		
		img1_sum_ptr1      = pss->img1_sum + temp;
		img2_sum_ptr1      = pss->img2_sum + temp;
		img1_sq_sum_ptr1   = pss->img1_sq_sum + temp;
		img2_sq_sum_ptr1   = pss->img2_sq_sum + temp;
		img12_mul_sum_ptr1 = pss->img12_mul_sum + temp;
		temp = y % 9 * stride;
		
		img1_sum_ptr2      = pss->img1_sum + temp;
		img2_sum_ptr2      = pss->img2_sum + temp;
		img1_sq_sum_ptr2   = pss->img1_sq_sum + temp;
		img2_sq_sum_ptr2   = pss->img2_sq_sum + temp;
		img12_mul_sum_ptr2 = pss->img12_mul_sum + temp;
		
		for (x = 0; x < width; x++)
		{
			img1_sum_ptr2[x]      = img1_sum_ptr1[x] + img1[x];
			img2_sum_ptr2[x]      = img2_sum_ptr1[x] + img2[x];
			img1_sq_sum_ptr2[x]   = img1_sq_sum_ptr1[x] + img1[x] * img1[x];
			img2_sq_sum_ptr2[x]   = img2_sq_sum_ptr1[x] + img2[x] * img2[x];
			img12_mul_sum_ptr2[x] = img12_mul_sum_ptr1[x] + img1[x] * img2[x];
		}
		
		if (y > 6)
		{
			//calculate the sum of the last 8 lines by subtracting the total sum of 8 lines back from the present sum
			temp = (y + 1) % 9 * stride;
			img1_sum_ptr1      = pss->img1_sum + temp;
			img2_sum_ptr1      = pss->img2_sum + temp;
			img1_sq_sum_ptr1   = pss->img1_sq_sum + temp;
			img2_sq_sum_ptr1   = pss->img2_sq_sum + temp;
			img12_mul_sum_ptr1 = pss->img12_mul_sum + temp;
			
			for (x = 0; x < width; x++)
			{
				img1_sum_ptr1[x]      = img1_sum_ptr2[x] - img1_sum_ptr1[x];
				img2_sum_ptr1[x]      = img2_sum_ptr2[x] - img2_sum_ptr1[x];
				img1_sq_sum_ptr1[x]   = img1_sq_sum_ptr2[x] - img1_sq_sum_ptr1[x];
				img2_sq_sum_ptr1[x]   = img2_sq_sum_ptr2[x] - img2_sq_sum_ptr1[x];
				img12_mul_sum_ptr1[x] = img12_mul_sum_ptr2[x] - img12_mul_sum_ptr1[x];
			}
			
			//here we calculate the sum over the 8x8 block of pixels
			//this is done by sliding a window across the column sums for the last 8 lines
			//each time adding the new column sum, and subtracting the one which fell out of the window
			img1_block      = 0;
			img2_block      = 0;
			img1_sq_block   = 0;
			img2_sq_block   = 0;
			img12_mul_block = 0;
			
			//prologue, and calculation of simularity measure from the first 8 column sums
			for (x = 0; x < 8; x++)
			{
				img1_block      += img1_sum_ptr1[x];
				img2_block      += img2_sum_ptr1[x];
				img1_sq_block   += img1_sq_sum_ptr1[x];
				img2_sq_block   += img2_sq_sum_ptr1[x];
				img12_mul_block += img12_mul_sum_ptr1[x];
			}
			planeQuality += similarity(img1_block, img2_block, img1_sq_block, img2_sq_block, img12_mul_block);
			
			//and for the rest
			for (x = 8; x < width; x++)
			{
				img1_block      = img1_block + img1_sum_ptr1[x] - img1_sum_ptr1[x - 8];
				img2_block      = img2_block + img2_sum_ptr1[x] - img2_sum_ptr1[x - 8];
				img1_sq_block   = img1_sq_block + img1_sq_sum_ptr1[x] - img1_sq_sum_ptr1[x - 8];
				img2_sq_block   = img2_sq_block + img2_sq_sum_ptr1[x] - img2_sq_sum_ptr1[x - 8];
				img12_mul_block = img12_mul_block + img12_mul_sum_ptr1[x] - img12_mul_sum_ptr1[x - 8];
				planeQuality += similarity(img1_block, img2_block, img1_sq_block, img2_sq_block, img12_mul_block);
			}
		}
	}
	
	if (planeSummedWeights == 0.0)
		return 1.0f;
	else
		return planeQuality / planeSummedWeights;
	
}



static void ssim_free(psnr_ssim_t *pss)
{
	if (pss->img12_sum_block != NULL)
	{
		free(pss->img12_sum_block);
		free(pss->img1_sum);
		free(pss->img2_sum);
		free(pss->img1_sq_sum);
		free(pss->img2_sq_sum);
		free(pss->img12_mul_sum);
		pss->img12_sum_block = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
static double calc_psnr_plane(byte *buf, int buf_stride, byte *ref, int ref_stride, int w, int h, int max_psnr, double *mse_total)
{
	int x, y, wh, aux;
	double mse, res;
	mse = 0.0;
	wh = w*h;
	if (wh <= 0) return 0.0;
	for (y=0; y<h; y++)
	{
		for (x=0; x<w; x++)
		{
			aux = buf[x] - ref[x];
			mse += aux * aux;
		}
		buf += buf_stride;
		ref += ref_stride;
	}
	if (mse < 1.0) mse = 1.0;
	mse /= (double)wh;
	(*mse_total) += mse;
	res = (10.0 * log10((255.*255.)/mse));
	if (max_psnr > 1 && res > max_psnr)
		res = max_psnr;
	return res;
}

static void ssim_alloc(psnr_ssim_t *pss, int w, int h)
{
	int w2 = w/2;
	int h2 = h/2;
	int stride = (w / 8 + 1) * 8;
	pss->img12_sum_block = NULL;
	if (w*h <= 0) return;
	pss->img12_sum_block = (short*)malloc(w2 * h2 * sizeof(short));
	pss->img1_sum      = (short *)malloc(9 * stride * sizeof(short));
	pss->img2_sum      = (short *)malloc(9 * stride * sizeof(short));
	pss->img1_sq_sum   = (int *)malloc(9 * stride * sizeof(int));
	pss->img2_sq_sum   = (int *)malloc(9 * stride * sizeof(int));
	pss->img12_mul_sum = (int *)malloc(9 * stride * sizeof(int));
}


//Public ----------------
static void psnr_ssim_init(psnr_ssim_t *pss, int max_psnr, int ssim_enable)
{
	assert(max_psnr > 0 || ssim_enable);
	pss->max_psnr = max_psnr;
	pss->ssim_enabled = ssim_enable;
	pss->num_frames = 0;
	pss->ssim_frame[0] = pss->ssim_frame[1] = pss->ssim_frame[2] = 0.0;
	pss->psnr_frame[0] = pss->psnr_frame[1] = pss->psnr_frame[2] = 0.0;
	pss->ssim_total[0] = pss->ssim_total[1] = pss->ssim_total[2] = 0.0;
	pss->psnr_total[0] = pss->psnr_total[1] = pss->psnr_total[2] = 0.0;
	pss->mse_total[0]  = pss->mse_total[1]  = pss->mse_total[2] = 0.0;
	pss->img12_sum_block = NULL;
}

static void convert_short2byte(byte  *dst,
							   short *src,
							   int    size,
							   int    bits)
{
	int i;
	for(i = 0; i < size; i += 1)
	{
		dst[i] = (byte)((src[i] >> (bits - 8)) & 0x00ff);
	}
}


static void convert_plane(byte *dst,
						  short *src,
						  int width,
						  int height,
						  int stride,
						  int bits)
{
	int y;
	for(y = 0; y < height; y += 1, dst += width, src += stride)
	{
		convert_short2byte(dst, src, width, bits);
	}
}


/**
* only planar formats supported so far: 4:0:0, 4:2:0, 4:2:2
*/
static void psnr_ssim_set_frame(psnr_ssim_t *pss, 
								int skip_frames, 
								sps_info_t *sps_info, 
								yuv_frame_t *dec_frame, 
								FILE *ref_stream)
{
	// luma variables:
	int lo, lw, lh, ls;
	// chroma variables:
	int co, cw, ch, cs, i, ysize, uvsize, ref_size, read_bytes;
	byte *refy;
	byte *refu;
	byte *refv;

	byte *tmp_y = NULL;
	byte *tmp_u = NULL;
	byte *tmp_v = NULL;
	int   fr_cr_height = 0;
	int src_bytes_per_pel = 1;

	if((sps_info->bit_depth_luma > 8) || (sps_info->bit_depth_chroma > 8))
	{
		src_bytes_per_pel = 2;
	}


	lo = sps_info->cropping_info.luma_offset;
	co = sps_info->cropping_info.chroma_offset;
	lw = sps_info->cropping_info.frame_width;
	lh = sps_info->cropping_info.frame_height;

	switch (sps_info->yuv_format)
	{
	case YUV_400_IDC:
		cw = 0;
		ch = 0;
		break;
	case YUV_422_IDC:
		cw = lw/2;
		ch = lh;
		fr_cr_height = dec_frame->height;
		break;
	default:
		cw = lw/2;
		ch = lh/2;
		fr_cr_height = dec_frame->height/2;
	}

	ls = dec_frame->stride_y;
	cs = dec_frame->stride_uv;

	// read next ref frame
	ysize  = (lw * lh * src_bytes_per_pel);
	uvsize = (cw * ch * src_bytes_per_pel);
	ref_size = (ysize + uvsize + uvsize);
	read_bytes;
	refy = (byte*) malloc(ref_size);
	refu = refy + ysize;
	refv = refu + uvsize;

	// skip selected number of ref frames
	for (i=0; i<skip_frames; i++)
	{
		read_bytes = (int)fread(refy, 1, ref_size, ref_stream);
	}
	read_bytes = (int)fread(refy, 1, ref_size, ref_stream);

	if (ref_size == read_bytes)
	{
		byte *y = (byte*)dec_frame->y;
		byte *u = (byte*)dec_frame->u;
		byte *v = (byte*)dec_frame->v;

		if(dec_frame->bytes_per_pel == 2)
		{
			int fr_y_size = dec_frame->bytes_per_pel * dec_frame->height * dec_frame->stride_y;
			int fr_u_size = dec_frame->bytes_per_pel * fr_cr_height * dec_frame->stride_uv;
			int fr_size = (fr_y_size + (2 * fr_u_size));

			tmp_y = (byte*) malloc(fr_size);
			tmp_u = tmp_y + fr_y_size;
			tmp_v = tmp_u + fr_u_size;

			memcpy(tmp_y, y, fr_y_size);
			if(sps_info->yuv_format != YUV_400_IDC)
			{
				memcpy(tmp_u, u, fr_u_size);
				memcpy(tmp_v, v, fr_u_size);
			}

			y = tmp_y;
			u = tmp_u;
			v = tmp_v;

			if(src_bytes_per_pel != 1) convert_short2byte((byte*)refy, (short*)refy,  (ysize/2), sps_info->bit_depth_luma);
			convert_plane(y + lo, (y + (lo * 2)), lw, lh, ls, sps_info->bit_depth_luma);

			ls = lw;

			if(sps_info->yuv_format != YUV_400_IDC)
			{
				if(src_bytes_per_pel != 1) convert_short2byte((byte*)refu, (short*)refu, (uvsize/2), sps_info->bit_depth_chroma);
				if(src_bytes_per_pel != 1) convert_short2byte((byte*)refv, (short*)refv, (uvsize/2), sps_info->bit_depth_chroma);

				convert_plane(u + co, (u + (co * 2)), cw, ch, cs, sps_info->bit_depth_chroma);
				convert_plane(v + co, (v + (co * 2)), cw, ch, cs, sps_info->bit_depth_chroma);
				cs = cw;
			}
		}

		if (pss->max_psnr)
		{
			pss->psnr_frame[0] = calc_psnr_plane(y+lo, ls, refy, lw, lw, lh, pss->max_psnr, &pss->mse_total[0]);
			pss->psnr_frame[1] = calc_psnr_plane(u+co, cs, refu, cw, cw, ch, pss->max_psnr, &pss->mse_total[1]);
			pss->psnr_frame[2] = calc_psnr_plane(v+co, cs, refv, cw, cw, ch, pss->max_psnr, &pss->mse_total[2]);
			pss->psnr_total[0] += pss->psnr_frame[0];
			pss->psnr_total[1] += pss->psnr_frame[1];
			pss->psnr_total[2] += pss->psnr_frame[2];
		}

		if (pss->ssim_enabled)
		{
			ssim_alloc(pss, lw, lh);
			pss->ssim_frame[0] = calc_ssim_plane(pss, y+lo, ls, refy, lw, lw, lh);
			pss->ssim_frame[1] = calc_ssim_plane(pss, u+co, cs, refu, cw, cw, ch);
			pss->ssim_frame[2] = calc_ssim_plane(pss, v+co, cs, refv, cw, cw, ch);
			ssim_free(pss);
			pss->ssim_total[0] += pss->ssim_frame[0];
			pss->ssim_total[1] += pss->ssim_frame[1];
			pss->ssim_total[2] += pss->ssim_frame[2];
		}
	}

	if(tmp_y != NULL)
	{
		free(tmp_y);
	}

	free(refy);
}

#endif //__PSNR_SSIM_H__
