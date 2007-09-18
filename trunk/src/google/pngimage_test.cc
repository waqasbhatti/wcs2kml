#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <google/color.h>
#include <google/pngimage.h>
#include <google/uint8.h>

namespace google_sky {

int Main(int argc, char **argv) {
  // Test Resize() and accessors.
  {
    PngImage image;
    int width = 5;
    int height = 10;
    PngImage::Colorspace colorspace;
    int channels;
    
    // Grayscale.
    colorspace = PngImage::GRAYSCALE;
    channels = 1;
    assert(image.Resize(width, height, colorspace));
    assert(image.width() == width);
    assert(image.height() == height);
    assert(image.colorspace() == colorspace);
    assert(image.channels() == channels);

    // Grayscale + alpha.
    colorspace = PngImage::GRAYSCALE_PLUS_ALPHA;
    channels = 2;
    assert(image.Resize(width, height, colorspace));
    assert(image.width() == width);
    assert(image.height() == height);
    assert(image.colorspace() == colorspace);
    assert(image.channels() == channels);

    // RGB.
    colorspace = PngImage::RGB;
    channels = 3;
    assert(image.Resize(width, height, colorspace));
    assert(image.width() == width);
    assert(image.height() == height);
    assert(image.colorspace() == colorspace);
    assert(image.channels() == channels);

    // RGBA.
    colorspace = PngImage::RGBA;
    channels = 4;
    assert(image.Resize(width, height, colorspace));
    assert(image.width() == width);
    assert(image.height() == height);
    assert(image.colorspace() == colorspace);
    assert(image.channels() == channels);
  }

  // Test getting & setting pixels.
  {
    PngImage image;
    int width = 5;
    int height = 10;
    PngImage::Colorspace colorspace = PngImage::RGB;
    assert(image.Resize(width, height, colorspace));

    Color write_pixel(image.channels());
    Color read_pixel(image.channels());
    write_pixel.SetAllChannels(128);
    
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.SetPixel(i, j, write_pixel);
        image.GetPixel(i, j, &read_pixel);
        assert(write_pixel.Equals(read_pixel));
      }
    }
  }

  // Test setting all values.
  {
    PngImage image;
    int width = 5;
    int height = 10;
    PngImage::Colorspace colorspace = PngImage::RGB;
    assert(image.Resize(width, height, colorspace));

    image.SetAllValues(34);
    Color pixel(image.channels());
    Color true_pixel(image.channels());
    true_pixel.SetAllChannels(34);
    
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &pixel);
        assert(pixel.Equals(true_pixel));
      }
    }
  }

  // Test setting individual channels.
  {
    PngImage image;
    int width = 5;
    int height = 10;
    PngImage::Colorspace colorspace = PngImage::GRAYSCALE_PLUS_ALPHA;
    assert(image.Resize(width, height, colorspace));

    assert(image.SetAllValuesInChannel(0, 34));
    assert(image.SetAllValuesInChannel(1, 68));
    Color pixel(image.channels());
    Color true_pixel(image.channels());
    true_pixel.SetChannel(0, 34);
    true_pixel.SetChannel(1, 68);
    
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &pixel);
        assert(pixel.Equals(true_pixel));
      }
    }
  }
  
  // Test colorspace conversions.
  {
    PngImage image;
    int width = 5;
    int height = 10;
    PngImage::Colorspace colorspace = PngImage::GRAYSCALE;
    assert(image.Resize(width, height, colorspace));

    // Every version of the image should be gray.  This doesn't test every
    // possible permutation of conversion.
    image.SetAllValues(128);

    Color g_pixel(1);
    Color g_pixel_true(1);
    g_pixel_true.SetAllChannels(128);

    Color ga_pixel(2);
    Color ga_pixel_true(2);
    ga_pixel_true.SetChannel(0, 128);
    ga_pixel_true.SetChannel(1, 255);

    Color rgb_pixel(3);
    Color rgb_pixel_true(3);
    rgb_pixel_true.SetAllChannels(128);
    
    Color rgba_pixel(4);
    Color rgba_pixel_true(4);
    rgba_pixel_true.SetChannels(0, 3, 128);
    rgba_pixel_true.SetChannel(3, 255);

    // Grayscale + alpha (adds an opaque alpha channel).
    assert(image.ConvertToGrayscalePlusAlpha());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &ga_pixel);
        assert(ga_pixel.Equals(ga_pixel_true));
      }
    }

    // RGBA (preserves alpha channel).
    assert(image.ConvertToRGBA());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &rgba_pixel);
        assert(rgba_pixel.Equals(rgba_pixel_true));
      }
    }

    // RGB.
    assert(image.ConvertToRGB());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &rgb_pixel);
        assert(rgb_pixel.Equals(rgb_pixel_true));
      }
    }

    // Grayscale.
    assert(image.ConvertToGrayscale());
    for (int i = 0; i < image.width(); ++i) {
      for (int j = 0; j < image.height(); ++j) {
        image.GetPixel(i, j, &g_pixel);
        assert(g_pixel.Equals(g_pixel_true));
      }
    }
  }
  
  // TODO: Test Read().
  // TODO: Test Write().
  
  return 0;
}

}  // end namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
