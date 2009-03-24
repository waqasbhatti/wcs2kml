// Copyright (c) 2007-2009, Google Inc.
// Author: Jeremy Brewer
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <cstdio>
#include <cassert>

#include <string>

#include "base.h"
#include "kml.h"
#include "string_util.h"

static const char *GROUND_OVERLAY =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<kml xmlns=\"http://earth.google.com/kml/2.2\" hint=\"target=sky\">\n"
    "<Document>\n"
    "  <GroundOverlay>\n"
    "    <name>A sample overlay</name>\n"
    "    <Icon>\n"
    "      <href>foo.png</href>\n"
    "    </Icon>\n"
    "    <LatLonBox>\n"
    "      <north>12.00000000000000</north>\n"
    "      <south>11.00000000000000</south>\n"
    "      <east>32.00000000000000</east>\n"
    "      <west>30.00000000000000</west>\n"
    "    </LatLonBox>\n"
    "    <LookAt>\n"
    "      <longitude>31.00000000000000</longitude>\n"
    "      <latitude>11.50000000000000</latitude>\n"
    "      <range>10.00000000000000</range>\n"
    "    </LookAt>\n"
    "  </GroundOverlay>\n"
    "</Document>\n"
    "</kml>\n";

static const char *ROOT_KML =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<kml xmlns=\"http://earth.google.com/kml/2.2\" hint=\"target=sky\">\n"
    "<Document>\n"
    "  <NetworkLink>\n"
    "    <name>A network link example</name>\n"
    "    <Region>\n"
    "      <LatLonAltBox>\n"
    "        <north>12.00000000000000</north>\n"
    "        <south>11.00000000000000</south>\n"
    "        <east>32.00000000000000</east>\n"
    "        <west>30.00000000000000</west>\n"
    "      </LatLonAltBox>\n"
    "      <Lod>\n"
    "        <minLodPixels>0</minLodPixels>\n"
    "        <maxLodPixels>256</maxLodPixels>\n"
    "      </Lod>\n"
    "    </Region>\n"
    "    <Link>\n"
    "      <href>foo.kml</href>\n"
    "    </Link>\n"
    "  </NetworkLink>\n"
    "</Document>\n"
    "</kml>\n";

static const char *TILE_KML =
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<kml xmlns=\"http://earth.google.com/kml/2.2\" hint=\"target=sky\">\n"
    "<Document>\n"
    "  <Region>\n"
    "    <LatLonAltBox>\n"
    "      <north>12.00000000000000</north>\n"
    "      <south>11.00000000000000</south>\n"
    "      <east>32.00000000000000</east>\n"
    "      <west>30.00000000000000</west>\n"
    "    </LatLonAltBox>\n"
    "    <Lod>\n"
    "      <minLodPixels>0</minLodPixels>\n"
    "      <maxLodPixels>256</maxLodPixels>\n"
    "    </Lod>\n"
    "  </Region>\n"
    "  <Placemark>\n"
    "    <LineString>\n"
    "      <coordinates>\n"
    "        32.00000000000000,12.00000000000000,0.00000000000000\n"
    "        30.00000000000000,12.00000000000000,0.00000000000000\n"
    "        30.00000000000000,11.00000000000000,0.00000000000000\n"
    "        32.00000000000000,11.00000000000000,0.00000000000000\n"
    "        32.00000000000000,12.00000000000000,0.00000000000000\n"
    "      </coordinates>\n"
    "    </LineString>\n"
    "  </Placemark>\n"
    "  <GroundOverlay>\n"
    "    <drawOrder>0</drawOrder>\n"
    "    <Icon>\n"
    "      <href>foo.png</href>\n"
    "    </Icon>\n"
    "    <LatLonBox>\n"
    "      <north>12.00000000000000</north>\n"
    "      <south>11.00000000000000</south>\n"
    "      <east>32.00000000000000</east>\n"
    "      <west>30.00000000000000</west>\n"
    "    </LatLonBox>\n"
    "  </GroundOverlay>\n"
    "  <NetworkLink>\n"
    "    <Region>\n"
    "      <LatLonAltBox>\n"
    "        <north>11.50000000000000</north>\n"
    "        <south>11.00000000000000</south>\n"
    "        <east>31.00000000000000</east>\n"
    "        <west>30.00000000000000</west>\n"
    "      </LatLonAltBox>\n"
    "      <Lod>\n"
    "        <minLodPixels>0</minLodPixels>\n"
    "        <maxLodPixels>256</maxLodPixels>\n"
    "      </Lod>\n"
    "    </Region>\n"
    "    <Link>\n"
    "      <href>foo_11.5_11.0_31.0_30.0.kml</href>\n"
    "    </Link>\n"
    "  </NetworkLink>\n"
    "  <NetworkLink>\n"
    "    <Region>\n"
    "      <LatLonAltBox>\n"
    "        <north>11.50000000000000</north>\n"
    "        <south>11.00000000000000</south>\n"
    "        <east>32.00000000000000</east>\n"
    "        <west>31.00000000000000</west>\n"
    "      </LatLonAltBox>\n"
    "      <Lod>\n"
    "        <minLodPixels>0</minLodPixels>\n"
    "        <maxLodPixels>256</maxLodPixels>\n"
    "      </Lod>\n"
    "    </Region>\n"
    "    <Link>\n"
    "      <href>foo_11.5_11.0_32.0_31.0.kml</href>\n"
    "    </Link>\n"
    "  </NetworkLink>\n"
    "  <NetworkLink>\n"
    "    <Region>\n"
    "      <LatLonAltBox>\n"
    "        <north>12.00000000000000</north>\n"
    "        <south>11.50000000000000</south>\n"
    "        <east>31.00000000000000</east>\n"
    "        <west>30.00000000000000</west>\n"
    "      </LatLonAltBox>\n"
    "      <Lod>\n"
    "        <minLodPixels>0</minLodPixels>\n"
    "        <maxLodPixels>256</maxLodPixels>\n"
    "      </Lod>\n"
    "    </Region>\n"
    "    <Link>\n"
    "      <href>foo_12.0_11.5_31.0_30.0.kml</href>\n"
    "    </Link>\n"
    "  </NetworkLink>\n"
    "  <NetworkLink>\n"
    "    <Region>\n"
    "      <LatLonAltBox>\n"
    "        <north>12.00000000000000</north>\n"
    "        <south>11.50000000000000</south>\n"
    "        <east>32.00000000000000</east>\n"
    "        <west>31.00000000000000</west>\n"
    "      </LatLonAltBox>\n"
    "      <Lod>\n"
    "        <minLodPixels>0</minLodPixels>\n"
    "        <maxLodPixels>256</maxLodPixels>\n"
    "      </Lod>\n"
    "    </Region>\n"
    "    <Link>\n"
    "      <href>foo_12.0_11.5_32.0_31.0.kml</href>\n"
    "    </Link>\n"
    "  </NetworkLink>\n"
    "</Document>\n"
    "</kml>\n";

namespace google_sky {

KmlNetworkLink MakeNetworkLink(double west, double south, double east,
                               double north) {
  string href =
      StringPrintf("foo_%.1f_%.1f_%.1f_%.1f.kml", north, south, east, west);
  
  KmlLatLonAltBox lat_lon_alt_box;
  lat_lon_alt_box.north.set(north);
  lat_lon_alt_box.south.set(south);
  lat_lon_alt_box.east.set(east);
  lat_lon_alt_box.west.set(west);

  KmlLod lod;
  lod.min_lod_pixels.set(0);
  lod.max_lod_pixels.set(256);

  KmlRegion region;
  region.lat_lon_alt_box.set(lat_lon_alt_box);
  region.lod.set(lod);

  KmlLink link;
  link.href.set(href);

  KmlNetworkLink network_link;
  network_link.region.set(region);
  network_link.link.set(link);

  return network_link;
}

int Main(int argc, char **argv) {
  // For this test, we will create 3 examples of the kinds of KML that 
  // wcs2kml generates and verify them.  A more rigorous test would check
  // every possible combination of tags, but that is clearly a lot of work.
  
  // Create a KML document containing a single Ground Overlay.
  {
    KmlIcon icon;
    icon.href.set("foo.png");

    KmlLatLonBox lat_lon_box;
    lat_lon_box.north.set(12.0);
    lat_lon_box.south.set(11.0);
    lat_lon_box.east.set(32.0);
    lat_lon_box.west.set(30.0);

    KmlLookAt look_at;
    look_at.latitude.set(11.5);
    look_at.longitude.set(31.0);
    look_at.range.set(10.0);

    KmlGroundOverlay ground_overlay;
    ground_overlay.name.set("A sample overlay");
    ground_overlay.icon.set(icon);
    ground_overlay.lat_lon_box.set(lat_lon_box);
    ground_overlay.look_at.set(look_at);

    Kml kml;
    kml.AddGroundOverlay(ground_overlay);

    // Get the raw XML string describing the KML.
    string kml_string = kml.ToString();
    assert(kml_string == GROUND_OVERLAY);
  }
  
  // Create a root KML as generated by Regionator.
  {
    KmlLatLonAltBox lat_lon_alt_box;
    lat_lon_alt_box.north.set(12.0);
    lat_lon_alt_box.south.set(11.0);
    lat_lon_alt_box.east.set(32.0);
    lat_lon_alt_box.west.set(30.0);

    KmlLod lod;
    lod.min_lod_pixels.set(0);
    lod.max_lod_pixels.set(256);

    KmlRegion region;
    region.lat_lon_alt_box.set(lat_lon_alt_box);
    region.lod.set(lod);

    KmlLink link;
    link.href.set("foo.kml");

    KmlNetworkLink network_link;
    network_link.name.set("A network link example");
    network_link.region.set(region);
    network_link.link.set(link);
      
    Kml kml;
    kml.AddNetworkLink(network_link);
    
    string kml_string = kml.ToString();
    assert(kml_string == ROOT_KML);
  }
  
  // Create a regionated til KML as generated by Regionator.
  {
    double north = 12.0;
    double south = 11.0;
    double east = 32.0;
    double west = 30.0;

    KmlLatLonAltBox lat_lon_alt_box;
    lat_lon_alt_box.north.set(north);
    lat_lon_alt_box.south.set(south);
    lat_lon_alt_box.east.set(east);
    lat_lon_alt_box.west.set(west);

    KmlLod lod;
    lod.min_lod_pixels.set(0);
    lod.max_lod_pixels.set(256);

    KmlRegion region;
    region.lat_lon_alt_box.set(lat_lon_alt_box);
    region.lod.set(lod);

    KmlIcon icon;
    icon.href.set("foo.png");

    KmlGroundOverlay ground_overlay;
    ground_overlay.draw_order.set(0);
    ground_overlay.icon.set(icon);
    ground_overlay.lat_lon_box.set(lat_lon_alt_box);

    // Line string for the border of this image.
    KmlLineString line_string;
    line_string.AddCoordinate(east, north);
    line_string.AddCoordinate(west, north);
    line_string.AddCoordinate(west, south);
    line_string.AddCoordinate(east, south);
    line_string.AddCoordinate(east, north);
    
    KmlPlacemark placemark;
    placemark.line_string.set(line_string);

    // Network links for subtiles.
    double x1 = west;
    double x2 = east;
    double y1 = south;
    double y2 = north;
    double xmid = 0.5 * (west + east);
    double ymid = 0.5 * (north + south);
    KmlNetworkLink ul = MakeNetworkLink(x1, y1, xmid, ymid);
    KmlNetworkLink ur = MakeNetworkLink(xmid, y1, x2, ymid);
    KmlNetworkLink ll = MakeNetworkLink(x1, ymid, xmid, y2);
    KmlNetworkLink lr = MakeNetworkLink(xmid, ymid, x2, y2);

    // Add everything to the KML.
    Kml kml;
    kml.region.set(region);
    kml.AddGroundOverlay(ground_overlay);
    kml.AddPlacemark(placemark);
    kml.AddNetworkLink(ul);
    kml.AddNetworkLink(ur);
    kml.AddNetworkLink(ll);
    kml.AddNetworkLink(lr);
    
    string kml_string = kml.ToString();
    assert(kml_string == TILE_KML);
  }
  
  return 0;
}

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
