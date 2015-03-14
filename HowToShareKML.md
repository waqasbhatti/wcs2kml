# Sharing KML #

The output of _wcs2kml_ is either a single KML document and a warped image or a hierarchy of such documents if regionation was turned on.  So now that you've run _wcs2kml_ and produced a cool overlay, how do you share it with the world?

For small images (< 500 kilbytes), you can probably get away with not regionating your image.  In this case, you can simply package the KML and image in a KMZ file.  Name the KML file `doc.kml` and create a zip archive with an extension `.kmz` containing both the image and the KML:
```
> zip my_kml_file.kmz doc.kml my_warped_image_file.png
```
Then you can just upload this KMZ to a web server and let users download it or create a KML file that contains a `<NetworkLink>` pointing to this KMZ on the web server.  See the [KML tutorial site](http://code.google.com/apis/kml/documentation/kml_tut.html) for more information.

For larger images, you will want to regionate the image using the `--regionate` flag.  The regionated output will look like this for a root KML named `root.kml` and a tile subdirectory named `tiles`:

Top level directory:
  * `root.kml`
  * `tiles`

Subdirectory:
  * many `tile_*.png` files
  * many `tile_*.kml` files

For an output image of size 1024 x 768, the root KML file will look like this:

```
<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://earth.google.com/kml/2.2" hint="target=sky">
<Document>
  <NetworkLink>
    <Region>
      <LatLonAltBox>
        <north>14.12054416992398</north>
        <south>13.83475756129926</south>
        <east>-179.63956968353423</east>
        <west>-179.85660711656763</west>
      </LatLonAltBox>
      <Lod>
        <minLodPixels>0</minLodPixels>
        <maxLodPixels>-1</maxLodPixels>
      </Lod>
    </Region>
    <Link>
      <href>tiles/tile_0_0_1024_768.kml</href>
    </Link>
  </NetworkLink>
</Document>
</kml>
```

(Note that the actual `<href>` filename will have indeces rounded up to the nearest multiple of the image tile size.)

If you want to put this document on a web server, you need to change the `<Link>` tag in root.kml so that it is not pointing to a local KML document.  To do this, you need to make the `tiles` directory available on a web server and then update the `<Link>` value.  If you have a web server at `http://foo.org`, you would move the `tiles` directory to the web server and update the value of `<href>` to:

```
    <Link>
      <href>http://foo.org/tiles/tile_0_0_1024_768.kml</href>
    </Link>
```

Finally, send out your root KML document for the world to see!  You can also add any other information to your root KML you want, such as placemarks, to include additional information you think users would find interesting.

A last word of advice:  Your experience may vary based on your network, but we have found that the default LOD value used by the code may need some tweaking for optimum performance.  The default value is 128 (which means that a region will be activated and bring up a higher resolution image if that region spans at least 128 pixels in your viewport).  This is excellent when the image pyramid is on local disk, but over the web it can result in heavier bandwidth usage than many users can support.  If you change the valude to 256 by adding `--regionate_min_lod_pixels 256` to your command line, you may see an improvement in performance.