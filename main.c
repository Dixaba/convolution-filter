#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  if (sizeof(unsigned int) != 4)
    {
      printf("Can't parse on this PC!!1\n");
      return -1;
    }

  FILE *f = NULL;
  ////////////////////////////
  ///                      ///
  ///   Reading BMP file   ///
  ///                      ///
  ////////////////////////////
  f = fopen("1.bmp", "rb");

  if (!f)
    {
      printf("Can't open file\n");
      return -1;
    }

  unsigned char magic[2];
  fread(magic, sizeof(unsigned char), 2, f);

  if (magic[0] != 'B' || magic[1] != 'M')
    {
      printf("Not a BMP file!\n");
      fclose(f);
      return -1;
    }

  unsigned int filesize, offset;
  fread(&filesize, sizeof(filesize), 1, f);
  fread(&offset, sizeof(offset), 1, f); // 4 useless zero bytes
  fread(&offset, sizeof(offset), 1, f);

  if (offset != 54)
    {
      printf("Not compatible BMP version!\n");
      fclose(f);
      return -1;
    }

  unsigned int headerSize;
  fread(&headerSize, sizeof(headerSize), 1, f);

  if (headerSize != 40)
    {
      printf("Not compatible BMP version!\n");
      fclose(f);
      return -1;
    }

  int width, height;
  fread(&width, sizeof(width), 1, f);
  fread(&height, sizeof(height), 1, f);
  unsigned short bitPerPixel;
  fread(&bitPerPixel, sizeof(bitPerPixel), 1, f); // 2 useless bytes
  fread(&bitPerPixel, sizeof(bitPerPixel), 1, f);

  if (bitPerPixel != 24)
    {
      printf("Not compatible pixel format!\n");
      fclose(f);
      return -1;
    }

  unsigned int compression;
  fread(&compression, sizeof(compression), 1, f);

  if (compression != 0)
    {
      printf("Not compatible compression!\n");
      fclose(f);
      return -1;
    }

  unsigned int pixelByteCount, resolutionX, resolutionY, tableCount;
  fread(&pixelByteCount, sizeof(pixelByteCount), 1, f);
  fread(&resolutionX, sizeof(resolutionX), 1, f);
  fread(&resolutionY, sizeof(resolutionY), 1, f);
  fread(&tableCount, sizeof(tableCount), 1, f);

  if (tableCount != 0)
    {
      printf("Not compatible color table!\n");
      fclose(f);
      return -1;
    }

  fread(&tableCount, sizeof(tableCount), 1, f); // zero color table contains 4 bytes anyway
  int padding = (4 - width * 3 % 4) % 4;
  unsigned char c; // For padding bytes only
  unsigned char **red, **green, **blue;
  red = malloc(height * sizeof(unsigned char *));
  green = malloc(height * sizeof(unsigned char *));
  blue = malloc(height * sizeof(unsigned char *));

  for (int i = 0; i < height; i++)
    {
      red[i] = malloc(width * sizeof(unsigned char));
      green[i] = malloc(width * sizeof(unsigned char));
      blue[i] = malloc(width * sizeof(unsigned char));
    }

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
        {
          fread(&red[i][j], sizeof(unsigned char), 1, f);
          fread(&green[i][j], sizeof(unsigned char), 1, f);
          fread(&blue[i][j], sizeof(unsigned char), 1, f);
        }

      for (int j = 0; j < padding; j++) // Padding bytes
        {
          fread(&c, sizeof(c), 1, f);
        }
    }

  fclose(f);
  ////////////////////////
  ///                  ///
  ///   Reading done   ///
  ///                  ///
  ////////////////////////
  {}
  ///////////////////////////////
  ///                         ///
  ///   Reading config file   ///
  ///                         ///
  ///////////////////////////////

  if (argc == 1)
    { f = fopen("config.txt", "rw"); }
  else
    { f = fopen(argv[1], "rt"); }

  if (!f)
    {
      printf("Can't open config file\n");
      return -1;
    }

  int kernelSize;
  fscanf(f, "%i", &kernelSize);

  if (kernelSize > width || kernelSize > height)
    {
      printf("Too small image!\n");
      return -1;
    }

  if ((kernelSize <= 0) || (kernelSize % 2 != 1))
    {
      printf("Incorrect kernel size - must be positive and odd\n");
      return -1;
    }

  double kernel[kernelSize][kernelSize];

  for (int i = 0; i < kernelSize; i++)
    {
      for (int j = 0; j < kernelSize; j++)
        {
          fscanf(f, "%lf ", &kernel[i][j]);
        }
    }

  fclose(f);
  char mode = '1';
  int equalCounter = 0;

  for (int i = 0; i < kernelSize; i++)
    {
      for (int j = 0; j < kernelSize; j++)
        {
          if (kernel[i][j] == kernel[j][i])
            { equalCounter++; }
        }
    }

  if (equalCounter != kernelSize * kernelSize)
    {
      mode = 'x';
    }

  int kernelMaxOffset = kernelSize / 2;
  ////////////////////////
  ///                  ///
  ///   Reading done   ///
  ///                  ///
  ////////////////////////
  double k, o;
  double max, min, value;
  max = 0;
  min = 255;
  width -= kernelMaxOffset * 2;
  height -= kernelMaxOffset * 2;
  double **newred, **newgreen, **newblue;
  double h, v;
  newred = malloc(height * sizeof(double *));
  newgreen = malloc(height * sizeof(double *));
  newblue = malloc(height * sizeof(double *));

  for (int i = 0; i < height; i++)
    {
      newred[i] = malloc(width * sizeof(double));
      newgreen[i] = malloc(width * sizeof(double));
      newblue[i] = malloc(width * sizeof(double));
    }

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
        {
          // Red
          {
            h = v = 0;

            for (int x = -kernelMaxOffset; x <= kernelMaxOffset; x++)
              {
                for (int y = -kernelMaxOffset; y <= kernelMaxOffset; y++)
                  {
                    h += 1.0 * kernel[kernelMaxOffset + x][kernelMaxOffset + y] * red[kernelMaxOffset + i + x][kernelMaxOffset + j + y];

                    if (mode == 'x')
                      {v += 1.0 * kernel[kernelMaxOffset + y][kernelMaxOffset + x] * red[kernelMaxOffset + i + x][kernelMaxOffset + j + y];}
                  }
              }

            if (mode == 'x')
              {
                value = sqrt(h * h + v * v);

                if (value > max)
                  {
                    max = value;
                  }
              }
            else
              {
                value = h;

                if (value > max)
                  {
                    max = value;
                  }

                if (value < min)
                  {
                    min = value;
                  }
              }

            newred[i][j] = value;
          }
          // Green
          {
            h = v = 0;

            for (int x = -kernelMaxOffset; x <= kernelMaxOffset; x++)
              {
                for (int y = -kernelMaxOffset; y <= kernelMaxOffset; y++)
                  {
                    h += 1.0 * kernel[kernelMaxOffset + x][kernelMaxOffset + y] * green[kernelMaxOffset + i + x][kernelMaxOffset + j + y];

                    if (mode == 'x')
                      {v += 1.0 * kernel[kernelMaxOffset + y][kernelMaxOffset + x] * green[kernelMaxOffset + i + x][kernelMaxOffset + j + y];}
                  }
              }

            if (mode == 'x')
              {
                value = sqrt(h * h + v * v);

                if (value > max)
                  {
                    max = value;
                  }
              }
            else
              {
                value = h;

                if (value > max)
                  {
                    max = value;
                  }

                if (value < min)
                  {
                    min = value;
                  }
              }

            newgreen[i][j] = value;
          }
          // Blue
          {
            h = v = 0;

            for (int x = -kernelMaxOffset; x <= kernelMaxOffset; x++)
              {
                for (int y = -kernelMaxOffset; y <= kernelMaxOffset; y++)
                  {
                    h += 1.0 * kernel[kernelMaxOffset + x][kernelMaxOffset + y] * blue[kernelMaxOffset + i + x][kernelMaxOffset + j + y];

                    if (mode == 'x')
                      {v += 1.0 * kernel[kernelMaxOffset + y][kernelMaxOffset + x] * blue[kernelMaxOffset + i + x][kernelMaxOffset + j + y];}
                  }
              }

            if (mode == 'x')
              {
                value = sqrt(h * h + v * v);

                if (value > max)
                  {
                    max = value;
                  }
              }
            else
              {
                value = h;

                if (value > max)
                  {
                    max = value;
                  }

                if (value < min)
                  {
                    min = value;
                  }
              }

            newblue[i][j] = value;
          }
        }
    }

  // Input color arrays are as garbage now - free memory!
  for (int i = 0; i < height; i++)
    {
      free(red[i]);
      free(green[i]);
      free(blue[i]);
    }

  free(red);
  free(green);
  free(blue);
  k = o = 0;

  for (int i = 0; i < kernelSize; i++)
    {
      for (int j = 0; j < kernelSize; j++)
        {
          k += kernel[i][j];
        }
    }

  if (k != 0)
    {
      k = 1.0 / k;
    }
  else
    {
      if (mode == 'x')
        {
          k = 255 / max;
          o = 0;
        }
      else
        {
          k = 255 / (max - min);
          o = min;
        }
    }

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
        {
          newred[i][j] -= o;
          newred[i][j] *= k;
          newgreen[i][j] -= o;
          newgreen[i][j] *= k;
          newblue[i][j] -= o;
          newblue[i][j] *= k;
        }
    }

  ////////////////////////////
  ///                      ///
  ///   Writing BMP file   ///
  ///                      ///
  ////////////////////////////
  f = fopen("2.bmp", "wb");

  if (!f)
    {
      printf("Can't open or create file\n");
      return -1;
    }

  // Calculate new pixel byte count and file size
  padding = (4 - width * 3 % 4) % 4;
  pixelByteCount = height * (width * 3 + padding);
  filesize = pixelByteCount + offset;
  // OKay, now we can write BMP
  fwrite(magic, sizeof(unsigned char), 2, f);
  fwrite(&filesize, sizeof(filesize), 1, f);
  unsigned int zero = 0;
  fwrite(&zero, sizeof(zero), 1, f);
  fwrite(&offset, sizeof(offset), 1, f);
  fwrite(&headerSize, sizeof(headerSize), 1, f);
  fwrite(&width, sizeof(width), 1, f);
  fwrite(&height, sizeof(height), 1, f);
  unsigned short smth = 1;
  fwrite(&smth, sizeof(smth), 1, f); // 2 useless bytes
  fwrite(&bitPerPixel, sizeof(bitPerPixel), 1, f);
  fwrite(&compression, sizeof(compression), 1, f);
  fwrite(&pixelByteCount, sizeof(pixelByteCount), 1, f);
  fwrite(&resolutionX, sizeof(resolutionX), 1, f);
  fwrite(&resolutionY, sizeof(resolutionY), 1, f);
  fwrite(&zero, sizeof(zero), 1, f);
  fwrite(&tableCount, sizeof(tableCount), 1, f); // zero color table contains 4 zero bytes
  unsigned char r, g, b;

  for (int i = 0; i < height; i++)
    {
      for (int j = 0; j < width; j++)
        {
          r = newred[i][j];
          fwrite(&r, sizeof(r), 1, f);
          g = newgreen[i][j];
          fwrite(&g, sizeof(g), 1, f);
          b = newblue[i][j];
          fwrite(&b, sizeof(b), 1, f);
        }

      for (int j = 0; j < padding; j++) // Padding bytes
        {
          fwrite(&c, sizeof(unsigned char), 1, f);
        }
    }

  // Output color arrays are as garbage now - free memory!
  for (int i = 0; i < height; i++)
    {
      free(newred[i]);
      free(newgreen[i]);
      free(newblue[i]);
    }

  free(newred);
  free(newgreen);
  free(newblue);
  fclose(f);
  ////////////////////////
  ///                  ///
  ///   Writing done   ///
  ///                  ///
  ////////////////////////
  return 0;
}
