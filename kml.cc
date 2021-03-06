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

#include "kml.h"

#include "boundingbox.h"
#include "string_util.h"

namespace {

const string TWO_SPACES("  ");
const double PI = 3.1415926535897931;
const double TINY_FLOAT_VALUE = 1.0e-8;
const double RADIUS_EARTH = 6378135.0;       // in meters
const double VIEWABLE_ANGULAR_SCALE = 50.0;  // in degrees

// Indents a string 2 spaces per level.
void Indent(string *str, int indent_level) {
  string spaces("");
  for (int i = 0; i < indent_level; ++i) {
    spaces.append(TWO_SPACES);
  }
  str->insert(0, spaces);
}

// Adds a tag to the given KML string, properly indenting it first.  A newline
// is added to the end of the tag.
void AppendTag(string *xml, const char *tag, int indent_level) {
  string tag_copy(tag);
  tag_copy.append("\n");
  Indent(&tag_copy, indent_level);
  xml->append(tag_copy);
}

// Generates KML representations for primitive types easily.
string CreateKml(const google_sky::KmlField<int> &kml_field,
                      const string &name,
                      int indent_level) {
  string xml = google_sky::StringPrintf("<%s>%d</%s>\n", name.c_str(),
                                        kml_field.get(), name.c_str());
  Indent(&xml, indent_level);
  return xml;
}

string CreateKml(const google_sky::KmlField<double> &kml_field,
                      const string &name, int indent_level) {
  string xml = google_sky::StringPrintf("<%s>%.14f</%s>\n", name.c_str(),
                                        kml_field.get(), name.c_str());
  Indent(&xml, indent_level);
  return xml;
}

string CreateKml(const google_sky::KmlField<string> &kml_field,
                      const string &name,
                      int indent_level) {
  string xml = google_sky::StringPrintf("<%s>%s</%s>\n", name.c_str(),
                                        kml_field.get().c_str(), name.c_str());
  Indent(&xml, indent_level);
  return xml;
}

}  // namespace

namespace google_sky {

// KmlIcon methods.
string KmlIcon::ToString(int indent_level) const {
  CHECK(href.has_value()) << "No href value";
  string xml;
  AppendTag(&xml, "<Icon>", indent_level);
  xml.append(CreateKml(href, "href", indent_level + 1));
  AppendTag(&xml, "</Icon>", indent_level);
  return xml;
}

// KmlLatLonBox methods.
string KmlLatLonBox::ToString(int indent_level) const {
  CHECK(north.has_value()) << "No north value";
  CHECK(south.has_value()) << "No south value";
  CHECK(east.has_value()) << "No east value";
  CHECK(west.has_value()) << "No west value";
  string xml;
  AppendTag(&xml, "<LatLonBox>", indent_level);
  xml.append(CreateKml(north, "north", indent_level + 1));
  xml.append(CreateKml(south, "south", indent_level + 1));
  xml.append(CreateKml(east, "east", indent_level + 1));
  xml.append(CreateKml(west, "west", indent_level + 1));
  if (rotation.has_value()) {
    xml.append(CreateKml(rotation, "rotation", indent_level + 1));
  }
  AppendTag(&xml, "</LatLonBox>", indent_level);
  return xml;
}

// KmlLatLonAltBox methods.
string KmlLatLonAltBox::ToString(int indent_level) const {
  CHECK(north.has_value()) << "No north value";
  CHECK(south.has_value()) << "No south value";
  CHECK(east.has_value()) << "No east value";
  CHECK(west.has_value()) << "No west value";
  string xml;
  AppendTag(&xml, "<LatLonAltBox>", indent_level);
  xml.append(CreateKml(north, "north", indent_level + 1));
  xml.append(CreateKml(south, "south", indent_level + 1));
  xml.append(CreateKml(east, "east", indent_level + 1));
  xml.append(CreateKml(west, "west", indent_level + 1));
  if (rotation.has_value()) {
    xml.append(CreateKml(rotation, "rotation", indent_level + 1));
  }
  if (min_altitude.has_value()) {
    xml.append(CreateKml(min_altitude, "minAltitude", indent_level + 1));
  }
  if (max_altitude.has_value()) {
    xml.append(CreateKml(max_altitude, "maxAltitude", indent_level + 1));
  }
  AppendTag(&xml, "</LatLonAltBox>", indent_level);
  return xml;
}

// KmlLookAt methods.
string KmlLookAt::ToString(int indent_level) const {
  CHECK(longitude.has_value()) << "No longitude";
  CHECK(latitude.has_value()) << "No latitude value";
  CHECK(range.has_value()) << "No range value";
  string xml;
  AppendTag(&xml, "<LookAt>", indent_level);
  xml.append(CreateKml(longitude, "longitude", indent_level + 1));
  xml.append(CreateKml(latitude, "latitude", indent_level + 1));
  xml.append(CreateKml(range, "range", indent_level + 1));
  AppendTag(&xml, "</LookAt>", indent_level);
  return xml;
}

// KmlLod methods.
string KmlLod::ToString(int indent_level) const {
  CHECK(min_lod_pixels.has_value()) << "No min_lod_pixels value";
  CHECK(max_lod_pixels.has_value()) << "No max_lod_pixels value";
  string xml;
  AppendTag(&xml, "<Lod>", indent_level);
  xml.append(CreateKml(min_lod_pixels, "minLodPixels", indent_level + 1));
  xml.append(CreateKml(max_lod_pixels, "maxLodPixels", indent_level + 1));
  AppendTag(&xml, "</Lod>", indent_level);
  return xml;
}

// KmlPoint methods.
string KmlPoint::ToString(int indent_level) const {
  CHECK(longitude.has_value()) << "No longitude";
  CHECK(latitude.has_value()) << "No latitude";
  // This is a special case CreateKml() can't handle.
  string coordinates =
      StringPrintf("<coordinates>%.14f,%.14f</coordinates>\n", longitude.get(),
                   latitude.get());
  Indent(&coordinates, indent_level + 1);
  string xml;
  AppendTag(&xml, "<Point>", indent_level);
  xml.append(coordinates);
  AppendTag(&xml, "</Point>", indent_level);
  return xml;
}

// KmlLineString methods.
string KmlLineString::ToString(int indent_level) const {
  CHECK_GT(longitudes_.size(), 0);
  CHECK_GT(latitudes_.size(), 0);
  string xml;
  AppendTag(&xml, "<LineString>", indent_level);
  AppendTag(&xml, "<coordinates>", indent_level + 1);
  string line;
  for (int i = 0; i < static_cast<int>(longitudes_.size()); ++i) {
    SStringPrintf(&line, "%.14f,%.14f,%.14f\n", longitudes_[i],
                  latitudes_[i], 0.0);
    Indent(&line, indent_level + 2);
    xml.append(line);
  }
  AppendTag(&xml, "</coordinates>", indent_level + 1);
  AppendTag(&xml, "</LineString>", indent_level);
  return xml;
}

// KmlGroundOverlay methods.
string KmlGroundOverlay::ToString(int indent_level) const {
  CHECK(lat_lon_box.has_value()) << "No lat_lon_box value";
  CHECK(icon.has_value()) << "No icon value";
  string xml;
  AppendTag(&xml, "<GroundOverlay>", indent_level);
  if (name.has_value()) {
    xml.append(CreateKml(name, "name", indent_level + 1));
  }
  if (draw_order.has_value()) {
    xml.append(CreateKml(draw_order, "drawOrder", indent_level + 1));
  }
  xml.append(icon.get().ToString(indent_level + 1));
  xml.append(lat_lon_box.get().ToString(indent_level + 1));
  if (look_at.has_value()) {
    xml.append(look_at.get().ToString(indent_level + 1));
  }
  AppendTag(&xml, "</GroundOverlay>", indent_level);
  return xml;
}

// Determines the LatLonBox and LookAt automatically from a BoundingBox
void KmlGroundOverlay::FromBoundingBox(const BoundingBox &bounding_box) {
  double ra_center;
  double dec_center;
  bounding_box.GetRaDecCenter(&ra_center, &dec_center);
  CHECK_GTE(ra_center, 0.0);
  CHECK_LT(ra_center, 360.0);
  
  // For the corners, we need to include the properly wrapped coordinates.
  double east;
  double west;
  bounding_box.GetWrappedRaBounds(&west, &east);
  double north;
  double south;
  bounding_box.GetDecBounds(&south, &north);

  // Now we can convert to the KML -180 to 180 coordinate system.
  // NB: The ra values here must be between 0 and 360 for this conversion to
  // be correct (otherwise it's -360).
  ra_center -= 180.0;
  east -= 180.0;
  west -= 180.0;

  // Google Earth assumes that east > west, otherwise it seems to switch
  // the coordinates (maybe it thinks you swapped them by mistake?).  For
  // some reason though, increasing east doesn't work, but decreasing west
  // does.
  if (east < west) {
    west -= 360.0;
  }

  // Determine the maximum angular scale spanned by the image.
  double ra_min;
  double ra_max;
  bounding_box.GetMonotonicRaBounds(&ra_min, &ra_max);
  double delta_ra = ra_max - ra_min;
  double delta_dec = north - south;
  double angular_scale;
  if (delta_ra > delta_dec) {
    angular_scale = delta_ra;
  } else {
    angular_scale = delta_dec;
  }

  // Determine the range to use so that the image spans a reasonable angular
  // scale when zoomed to.  The max viewable angular scale, alpha, was chosen
  // by trying various values until one looked nice.  We must be careful that
  // angular scales > VIEWABLE_ANGULAR_SCALE zoom all the way out; hence beta
  // has a maximum value of alpha.
  // NB: The maximum zoom range in the client is actually 1.5 * RADIUS_EARTH,
  //     but user KML can only zoom out to RADIUS_EARTH.
  double alpha = 0.5 * VIEWABLE_ANGULAR_SCALE * (PI / 180.0);
  double beta = 0.5 * angular_scale * (PI / 180.0);
  if (beta > alpha) {
    beta = alpha;
  }
  double range = RADIUS_EARTH * (1.0 - (sin(alpha - beta) /
                                       (sin(alpha) + TINY_FLOAT_VALUE)));

  KmlLatLonBox new_lat_lon_box;
  new_lat_lon_box.north.set(north);
  new_lat_lon_box.south.set(south);
  new_lat_lon_box.east.set(east);
  new_lat_lon_box.west.set(west);

  KmlLookAt new_look_at;
  new_look_at.longitude.set(ra_center);
  new_look_at.latitude.set(dec_center);
  new_look_at.range.set(range);

  lat_lon_box.set(new_lat_lon_box);
  look_at.set(new_look_at);
}

// KmlRegion methods.
string KmlRegion::ToString(int indent_level) const {
  CHECK(lat_lon_alt_box.has_value()) << "No lat_lon_alt_box value";
  string xml;
  AppendTag(&xml, "<Region>", indent_level);
  xml.append(lat_lon_alt_box.get().ToString(indent_level + 1));
  if (lod.has_value()) {
    xml.append(lod.get().ToString(indent_level + 1));
  }
  AppendTag(&xml, "</Region>", indent_level);
  return xml;
}

// KmlLink methods.
string KmlLink::ToString(int indent_level) const {
  CHECK(href.has_value()) << "No href value";
  string xml;
  AppendTag(&xml, "<Link>", indent_level);
  xml.append(CreateKml(href, "href", indent_level + 1));
  AppendTag(&xml, "</Link>", indent_level);
  return xml;
}

// KmlNetworkLink methods.
string KmlNetworkLink::ToString(int indent_level) const {
  CHECK(link.has_value()) << "No link value";
  string xml;
  AppendTag(&xml, "<NetworkLink>", indent_level);
  if (name.has_value()) {
    xml.append(CreateKml(name, "name", indent_level + 1));
  }
  if (region.has_value()) {
    xml.append(region.get().ToString(indent_level + 1));
  }
  xml.append(link.get().ToString(indent_level + 1));
  AppendTag(&xml, "</NetworkLink>", indent_level);
  return xml;
}

// KmlPlacemark methods.
string KmlPlacemark::ToString(int indent_level) const {
  CHECK(name.has_value() || description.has_value() ||
        point.has_value() || line_string.has_value())
      << "Placemarks must have a name, description, point, or line_string";
  string xml;
  AppendTag(&xml, "<Placemark>", indent_level);
  if (name.has_value()) {
    xml.append(CreateKml(name, "name", indent_level + 1));
  }
  if (description.has_value()) {
    xml.append(CreateKml(description, "description", indent_level + 1));
  }
  if (look_at.has_value()) {
    xml.append(look_at.get().ToString(indent_level + 1));
  }
  if (point.has_value()) {
    xml.append(point.get().ToString(indent_level + 1));
  }
  if (line_string.has_value()) {
    xml.append(line_string.get().ToString(indent_level + 1));
  }
  AppendTag(&xml, "</Placemark>", indent_level);
  return xml;
}

// Kml methods.
string Kml::ToString(void) const {
  string xml;
  StringAppendF(&xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  StringAppendF(&xml, "<kml xmlns=\"http://earth.google.com/kml/2.2\" "
                "hint=\"target=sky\">\n");
  StringAppendF(&xml, "<Document>\n");

  // We let the <Document> tag carry the Region, which means that the Region
  // will cascade to all children (the arrays of Placemarks, GroundOverlays,
  // and NetworkLinks).  NetworkLinks therefore are expected to define their
  // own Regions.
  if (region.has_value()) {
    xml.append(region.get().ToString(1));
  }
  for (int i = 0; i < static_cast<int>(placemarks_.size()); ++i) {
    xml.append(placemarks_[i].ToString(1));
  }
  for (int i = 0; i < static_cast<int>(ground_overlays_.size()); ++i) {
    xml.append(ground_overlays_[i].ToString(1));
  }
  for (int i = 0; i < static_cast<int>(network_links_.size()); ++i) {
    xml.append(network_links_[i].ToString(1));
  }
  StringAppendF(&xml, "</Document>\n");
  StringAppendF(&xml, "</kml>\n");
  return xml;
}

}  // namespace google_sky
