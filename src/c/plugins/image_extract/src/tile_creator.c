#include "image_extract.h"

void set_grayscale(unsigned char *ptr, float val)
{
    ptr[0] = (unsigned char) ((int) val & 0xFF);
    ptr[1] = (unsigned char) ((int) val >> 8);
}

int		get_dim_size(t_vol *volume, char c)
{
  int		i = 0;

  while (i < volume->dim_nb)
    {
      if (volume->dim_name_char[i] == c)
	return (volume->size[i]);
      i++;
    }
  return (0);
}

void		get_width_height(int *height, int *width, int current_dimension,
				 t_vol *volume)
{
  if (volume->dim_name_char[current_dimension] == 'x')
    {
      *height = get_dim_size(volume, 'z'); //volume->size[Y];
      *width = get_dim_size(volume, 'y'); //volume->size[Z];
    }
  else if (volume->dim_name_char[current_dimension] == 'y')
    {
      *height = get_dim_size(volume, 'z');//volume->size[X];
      *width = get_dim_size(volume, 'x');//volume->size[Z];
    }
  else
    {
      *height = get_dim_size(volume, 'y');//volume->size[X];
      *width = get_dim_size(volume, 'x');//volume->size[Y];
    }
}

int check_pixels_range(int width, int height, int h_position, int w_position,
        int h_position_end, int w_position_end)
{
    if (h_position > height || w_position > width) {
        printf("X - Y coordinates out of range\n");
        return (1);
    }
    if (h_position_end > height || w_position_end > width) {
        printf("X - Y coordinates out of range\n");
        return (2);
    }
    if (h_position < 0 || w_position < 0 || h_position_end < 0
            || w_position_end < 0) {
        printf("Negative X - Y coordinates\n");
        return (3);
    }
    if (h_position > h_position_end || w_position > w_position_end) {
        printf("X - Y coordinates bigger than X_END - Y_END coordinates\n");
        return (4);
    }
    return (0);
}

int check_and_set_position(int kind, int width, int height, t_image_args *a)
{
    int i;

    width *= a->info->scale;
    height *= a->info->scale;
    i = 2;
    while (i == 2) {
    	if (a->info->w_position > width) {
    		a->info->w_position = a->info->start_w;
    	}
    	if (a->info->h_position > height) {
    		a->info->h_position = a->info->start_h;
    	}
        if (kind == 2)
            return (0);
        if (kind == 1) {
            a->info->w_position_end = a->info->w_position
                    + a->info->square_size;
            a->info->h_position_end = a->info->h_position
                    + a->info->square_size;
        }
        if (a->info->w_position_end > width)
            a->info->w_position_end = width;
        if (a->info->h_position_end > height)
            a->info->h_position_end = height;
        i = check_pixels_range(width, height, a->info->h_position,
                a->info->w_position, a->info->h_position_end,
                a->info->w_position_end);
    }
    if (i != 0)
        return (1);
    return (0);
}

int set_service_type(t_image_args * a)
{
    if (strcmp(a->info->service, "tiles") == 0)
        return (1);
    if (strcmp(a->info->service, "full") == 0)
        return (2);
    if (strcmp(a->info->service, "images") == 0)
        return (3);
    return (0);
}

RectangleInfo *create_rectangle_crop(int kind, t_image_args *a)
{
    RectangleInfo *portion;

    if (kind == 1 || kind == 3) {
        portion = malloc(sizeof(*portion));
        portion->width = (
			  kind == 1 ?
			  a->info->square_size :
			  a->info->w_position_end - a->info->w_position);
        portion->height = (
			   kind == 1 ?
			   a->info->square_size :
			   a->info->h_position_end - a->info->h_position);
        portion->x = a->info->w_position;
        portion->y = a->info->h_position;
        return (portion);
    }
    return (NULL);
}

void convert_tiles_to_pixel_coord(t_image_args *a)
{
    a->info->h_position *= a->info->square_size;
    a->info->w_position *= a->info->square_size;
}

void fclose_check(FILE *file) {
	if (file && fcntl(fileno(file), F_GETFL) != -1) {
        fclose(file);
    }
}

void		apply_colormap(PixelPacket *px, PixelPacket *px_final, float **premapped_colormap,
			       t_image_args *a, int width, int height)
{
  int		i = 0;
  int		j = 0;
  unsigned char	pixel_value;

  while (i < height)
    {
      j = 0;
      while (j < width)
	{
	  pixel_value = px[(width * i) + j].red;
	  pixel_value = (pixel_value >= 255 ? 254 : pixel_value);

	  px_final[(width * i) + j].red = (unsigned short)(premapped_colormap[pixel_value][0]);
	  px_final[(width * i) + j].green = (unsigned short)(premapped_colormap[pixel_value][1]);
	  px_final[(width * i) + j].blue = (unsigned short)(premapped_colormap[pixel_value][2]);
	  j++;
	}
      i++;
    }
}

unsigned char	get_contrasted_value(unsigned char min, unsigned char max,
				     unsigned char dataset_min, unsigned char dataset_max, unsigned char initial_value)
{
  float	ratio;
  unsigned char	final_value;

  ratio = (float)((float)((float)max - (float)min) / (float)((float)dataset_max - (float)dataset_min));
  final_value = min + round(initial_value * ratio);
  return (final_value);
}

void		apply_contrast(PixelPacket *px, unsigned char min, unsigned char max,
			       unsigned char dataset_min, unsigned char dataset_max,
			       t_image_args *a, int width, int height)
{
  int		i = 0;
  int		j = 0;
  unsigned char	pixel_value;

  while (i < height)
    {
      j = 0;
      while (j < width)
	{
	  pixel_value = px[(width * i) + j].red;
	  pixel_value = (pixel_value >= 255 ? 254 : pixel_value);

	  pixel_value = get_contrasted_value(min, max, dataset_min, dataset_max, pixel_value);

	  px[(width * i) + j].red = (unsigned char)(pixel_value);
	  px[(width * i) + j].green = (unsigned char)(pixel_value);
	  px[(width * i) + j].blue = (unsigned char)(pixel_value);
	  j++;
	}
      i++;
    }
}

void		print_image(char *hyperslab, t_vol *volume, int current_dimension,
			    unsigned int current_slice, int width, int height, t_image_args *a)
{
  ExceptionInfo exception;
  Image		*img;
  Image		*tmp;
  RectangleInfo *portion;
  ImageInfo	*image_info;
  int		kind;
  short		streamToSocket;
  Image		*new_image;
  ImageInfo	*new_image_info;
  ImageInfo	*image_info_cpy;
  PixelPacket	*px;
  PixelPacket	*px_tmp;


  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  streamToSocket = a->file && fcntl(fileno(a->file), F_GETFL) != -1;

  kind = set_service_type(a);

  convert_tiles_to_pixel_coord(a);

  if (check_and_set_position(kind, width, height, a)) {
    fclose_check(a->file);
    return;
  }

  portion = create_rectangle_crop(kind, a);

  GetExceptionInfo(&exception);
  if ((image_info = CloneImageInfo((ImageInfo *)NULL)) == NULL) {
    CatchException(&exception);
    DestroyImageInfo(image_info);
    fclose_check(a->file);
    return;
  }

  if ((img = ConstituteImage(width, height, "I", CharPixel, hyperslab,
			     &exception)) == NULL) {
    CatchException(&exception);
    DestroyImageInfo(image_info);
    fclose_check(a->file);
    return;
  }

  if (a->info->contrast != 0)
    {
      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      if ((px = GetImagePixelsEx(img, 0, 0, width, height, &exception)) == NULL)
	{
	  DestroyImage(img);
	  DestroyImageInfo(image_info);
	  fclose_check(a->file);
	  CatchException(&exception);
	  return;
	}

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      apply_contrast(px, a->info->contrast_min, a->info->contrast_max,
		     a->volume->color_range_min, a->volume->color_range_max, a, width, height);

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      SyncImagePixels(img);

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

    }

  if (a->info->colormap_id > -1)
    {
      if ((new_image_info = CloneImageInfo((ImageInfo *)NULL)) == NULL) {
	CatchException(&exception);
	DestroyImageInfo(image_info);
	DestroyImageInfo(new_image_info);
	DestroyImage(img);
	fclose_check(a->file);
	return;
      }

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      new_image_info->colorspace = RGBColorspace;
      new_image = AllocateImage(new_image_info);
      new_image->rows = height;
      new_image->columns = width;

      if ((px = GetImagePixelsEx(img, 0, 0, width, height, &exception)) == NULL)
	{
	  DestroyImage(img);
	  DestroyImageInfo(image_info);
	  DestroyImageInfo(new_image_info);
	  fclose_check(a->file);
	  CatchException(&exception);
	  return;
	}

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	DestroyImageInfo(new_image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      if ((px_tmp = SetImagePixelsEx(new_image, 0, 0, width, height, &exception)) == NULL)
	{
	  DestroyImage(img);
	  DestroyImageInfo(image_info);
	  DestroyImageInfo(new_image_info);
	  fclose_check(a->file);
	  CatchException(&exception);
	  return;
	}

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(new_image_info);
	DestroyImageInfo(image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      apply_colormap(px, px_tmp, a->info->premapped_colormap[a->info->colormap_id], a, width, height);

      if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
	DestroyImage(img);
	DestroyImageInfo(image_info);
	DestroyImageInfo(new_image_info);
	write_http_header(a->file, "408 Request Timeout", a->info->image_type);
	fclose_check(a->file);
	return;
      }

      SyncImagePixels(new_image);

      DestroyImage(img);
      image_info_cpy = image_info;
      image_info = new_image_info;
      DestroyImageInfo(image_info_cpy);
      img = new_image;
    }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  tmp = img;
  if ((img = FlipImage(img, &exception)) == NULL) {
    CatchException(&exception);
    DestroyImage(tmp);
    DestroyImageInfo(image_info);
    fclose_check(a->file);
    return;
  }
  DestroyImage(tmp);

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (a->info->quality != 1) {
    tmp = img;
    if ((img = SampleImage(img, width / a->info->quality,
			   height / a->info->quality, &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
    tmp = img;
    if ((img = SampleImage(img, width, height, &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (a->info->scale != 1) {
    tmp = img;
    if ((img = ScaleImage(img, (width * a->info->scale),
			  (height * a->info->scale), &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  if (kind == 1 || kind == 3) {
    tmp = img;
    if ((img = CropImage(img, portion, &exception)) == NULL) {
      CatchException(&exception);
      DestroyImage(tmp);
      DestroyImageInfo(image_info);
      fclose_check(a->file);
      return;
    }
    DestroyImage(tmp);
  }

  if (a->general_info->tile_requests->is_expired(a->general_info->tile_requests, a->info->request_id, a->info->request_time)) {
    DestroyImage(img);
    DestroyImageInfo(image_info);
    write_http_header(a->file, "408 Request Timeout", a->info->image_type);
    fclose_check(a->file);
    return;
  }

  // write image
  if (streamToSocket) { // SOCKET STREAM
    strcpy(img->magick, a->info->image_type);
    image_info->file = a->file;

    write_http_header(a->file, "200 OK", a->info->image_type);
    WriteImage(image_info, img);

    // clean up
    DestroyImage(img);
    DestroyImageInfo(image_info);
    fclose_check(a->file);
  }
  else
    { // WRITE FILE
      a->info->image_type = strlower(a->info->image_type);

      if (!a->info->root_path) {
	printf("Error: root path is NULL\n");
	return;
      }

      char dir[200]; // first path
      sprintf(dir, "%s/%c/%i", a->info->root_path, volume->dim_name[current_dimension][0], current_slice);
      t_string_buffer * finalPath = createDirectory(dir, 0777);
      if (finalPath == NULL) {
	return;
      }

      // complete filename
      if (strcmp(a->info->service, "full") == 0 && a->info->quality != 1) {
	sprintf(dir, "/%i.low.res.%s", current_slice, a->info->image_type);
      } else {
	sprintf(dir, "/%i_%i.%s", a->info->start_w, a->info->start_h, a->info->image_type);
      }
      finalPath = appendToBuffer(finalPath, dir);
      strcpy(img->filename, finalPath->buffer);

      WriteImage(image_info, img);

      DestroyImage(img);
      DestroyImageInfo(image_info);

      free(finalPath->buffer);
      free(finalPath);
    }
}
