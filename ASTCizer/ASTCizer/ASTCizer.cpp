#include <astcenc.h>
#include <astcenc_internal.h>

#include <cassert>
#include <cstring>
#include <string>
#include <vector>



int handle_error(astcenc_error status, std::string message = "")
{
  if (status == ASTCENC_ERR_BAD_BLOCK_SIZE)
  {
	printf("ERROR: %s. Block size is invalid\n", message);
	return 1;
  }
  else if (status == ASTCENC_ERR_BAD_CPU_ISA)
  {
	printf("ERROR: %s. Required SIMD ISA support missing on this CPU\n", message);
	return 1;
  }
  else if (status == ASTCENC_ERR_BAD_CPU_FLOAT)
  {
	printf("ERROR: %s. astcenc must not be compiled with -ffast-math\n", message);
	return 1;
  }
  else if (status != ASTCENC_SUCCESS)
  {
	printf("ERROR: %s. Failed with: %s\n", message, astcenc_get_error_string(status));
	return 1;
  }
  return 0;
}






static astcenc_image* load_hdf5_file(const char* filename,
  unsigned int dim_z,
  unsigned int dim_pad,
  bool y_flip,
  bool& is_hdr,
  unsigned int& num_components)
{
  astcenc_image* image = nullptr;

  
}




int edit_config(astcenc_config &config)
{
  /* ERROR WEIGHTING OPTIONS */
  /* V-MODE
   * Compute the per-texel relative error weighting for the RGB color channels as follows:
   * weight = 1 / (<base> + <mean> * mean^2 + <stdev> * stdev^2)
   */

  // Specifies radius of neighbourhood over which average abd standard deviation are computed
  config.v_rgba_radius;				// (uint) The size of the texel kernel for error weighting (-v).

  // Strict per colorchannel computations; 1: Fully mixed colorchannel computations.
  config.v_rgba_mean_stdev_mix;		// (float, [0..1]) The mean and stdev channel mix for error weighting (-v).

  // Take the input texels to the power of specified value before computing average and standard deviation.
  config.v_rgb_power;				// (float) The texel RGB power for error weighting (-v). 

  config.v_rgb_base;				// (float) The texel RGB base weight for error weighting (-v).

  config.v_rgb_mean;				// (float) The texel RGB mean weight for error weighting (-v).

  config.v_rgb_stdev;				// (float) The texel RGB stdev for error weighting (-v).


  /* ===================================================================== */
  /* VA-MODE
   * Compute the per-texel relative error weighting for the alpha channel, when used in conjunction with -v
   */

  config.v_a_power;		  // (float) The texel A power for error weighting (-va).

  config.v_a_base;		  // (float) The texel A base weight for error weighting (-va).

  config.v_a_mean;		  // (float) The texel A mean weight for error weighting (-va).

  config.v_a_stdev;		  // (float) The texel A stdev for error weighting (-va).


  /* ===================================================================== */
  /* CW-MODE
   * Assign an additional weight scaling to each color channel,
   * allowing the channels to be treated differently in terms of
   * error significance (>1: increased sig, <1 decreased sig).
   */
  config.cw_r_weight;	  // (float) The red channel weight scale for error weighting (-cw).

  config.cw_g_weight;	  // (float) The green channel weight scale for error weighting (-cw).

  config.cw_b_weight;	  // (float) The blue channel weight scale for error weighting (-cw).

  config.cw_a_weight;	  // (float) The alpha channel weight scale for error weighting (-cw).


  /* ===================================================================== */
  /* A-MODE
   * For textures with alpha channel, scale per-texel weights by the alpha value.
   * Recommendation: 1 if FLG_USE_ALPHA_WEIGHT is used.
   */
  unsigned int a_scale_radius;		// The radius for any alpha-weight scaling (-a).


  /* ===================================================================== */
  /* B-MODE
   * Assign an additional weight scaling for texels at compression block edges and corners.
   * This is generic tool for reducing artefacts visible on block changes (for values >1).
   */
  float b_deblock_weight;			//  The additional weight for block edge texels (-b).




  /* ===================================================================== */
  /* ===================================================================== */
  /* PERFORMANCE-QUALITY TRADEOFF*/
  /* Test only <number> block partitions. Higher numbers give
   * better quality, however large values give diminishing returns
   * especially for smaller block sizes.
   */
  unsigned int tune_partition_limit;	// The maximum number of partitions searched (-partitionlimit).

  /* Test only block modes below the <number> usage centile in an
   * empirically determined distribution of block mode frequency.
   * This option is ineffective for 3D textures.
   */
  unsigned int tune_block_mode_limit;	// The maximum centile for block modes searched (-blockmodelimit).

  /* Iterate only <value> refinement iterations on colors and
   * weights. Minimum value is 1.
   */
  unsigned int tune_refinement_limit;	// The maximum iterative refinements applied (-refinementlimit).

  /*
   * Stop compression work on a block as soon as the PSNR of the
   * block, measured in dB, exceeds <number>. This option is
   * ineffective for HDR textures.
   */
  float tune_db_limit;				  //  The dB threshold for stopping block search (-dblimit).

  /*
   * Stop compression work on a block after only testing blocks with
   * up to two partions and one plane of weights, unless the two
   * partition error term is lower than the error term from encoding
   * with one partition by more than the specified factor. This
   * option is ineffective for normal maps.
   */
  float tune_partition_early_out_limit;		// The threshold for skipping 3+ partitions (-partitionearlylimit).

  /**
   * Stop compression after testing only one planes of weights,
   * unless the minimum color correlation factor between any pair of
   * color channels is below this factor. This option is ineffective
   * for normal maps.
   */
  float tune_two_plane_early_out_limit;		// The threshold for skipping 2 weight planess (-planecorlimit).
}
















int main()
{
  astcenc_config my_config{};	// ToDo: Why is this struct initilized empty?
  
  /* BLOCK SIZES
   * For compression. block_z is for 3D images.
   * ToDo: Hardcode or argument
   */
  unsigned int block_x = 0;
  unsigned int block_y = 0;
  unsigned int block_z = 0;

  /* ENCODING FLAGS
   * Additional instructions for compression.
   */
  unsigned int flags = 0;
  flags |= ASTCENC_FLG_DECOMPRESS_ONLY;

  /* CONFIG INIT
   * Set Color Profile (Full HDR for 16bit float),
   *     Blocksizes,
   *     Encoding Intensity (Fast for Testing),
   *     Compressor Flags,
   *     Config-Struct.
   */
  astcenc_error status = astcenc_config_init(ASTCENC_PRF_HDR, block_x, block_y, block_z, ASTCENC_PRE_FAST, flags, my_config);
  if (handle_error(status, "config init failed."))
	return 1;

  // Add padding to the data according to the settings so far
  int padding = MAX(my_config.v_rgba_radius, my_config.a_scale_radius);

  

  /* LOAD IMAGE
   * The input can be either 8-bit unorm (LDR, data8-pointer) or 16-bit floating point (HDR, data16-pointer).
   * The unused pointer must be set to nullptr.
   */
  astcenc_image* image_uncomp_in = nullptr;
  unsigned int image_uncomp_in_channelcount = 3;
  bool image_uncomp_in_is_hdr = true;

  /*
  astcenc_image my_input;
  my_input.dim_x   = 0;
  my_input.dim_y   = 0;
  my_input.dim_z   = 0;
  my_input.dim_pad = 0;
  my_input.data16  = NULL;
  my_input.data8   = NULL;

  astcenc_swizzle swizzle;
  uint8_t* my_output;
  size_t len;
  */

  
  
  /* COMPRESS
   * The integer is the threadcount used for encoding.
   */
  astcenc_error    codec_status;
  astcenc_context* codec_context;
  astcenc_context_alloc(my_config, 1, &codec_context);
  if (handle_error(status, "context_alloc failed."))
	return 1;


  /*
  astcenc_compress_image(codec_context, my_input, swizzle, my_output, len, 1);
  */
  /* CLEANUP */
  astcenc_compress_reset(codec_context);
  astcenc_context_free(codec_context);


}