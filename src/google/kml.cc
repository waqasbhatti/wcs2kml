// Copyright 2007 Google Inc.
// Author: Jeremy Brewer
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <google/kml.h>

#include <google/boundingbox.h>
#include <google/stringprintf.h>

namespace {

const int STR_BUFSIZE = 1024;
const std::string TWO_SPACES("  ");
const double PI = 3.1415926535897931;
const double TINY_FLOAT_VALUE = 1.0e-8;
const double RADIUS_EARTH = 6378135.0;       // in meters
const double VIEWABLE_ANGULAR_SCALE = 50.0;  // in degrees

// Indents a string 2 spaces per level.
void Indent(std::string *str, int indent_level) {
  std::string spaces("");
  for (int i = 0; i < indent_level; ++i) {
    spaces.append(TWO_SPACES);
  }
  str->insert(0, spaces);
}

// Adds a tag to the given KML string, properly indenting it first.  A newline
// is added to the end of the tag.
void AppendTag(std::string *xml, const char *tag, int indent_level) {
  std::string tag_copy(tag);
  tag_copy.append("\n");
  Indent(&tag_copy, indent_level);
  xml->append(tag_copy);
}

// Generates KML representations for primitive types easily.
std::string CreateKml(const google_sky::KmlField<int> &kml_field,
                      const std::string &name,
                      int indent_level) {
  std::string xml;
  google_sky::StringPrintf(&xml, STR_BUFSIZE, "<%s>%d</%s>\n", name.c_str(),
                          kml_field.get(), name.c_str());
  Indent(&xml, indent_level);
  return xml;
}

std::string CreateKml(const google_sky::KmlField<double> &kml_field,
                      const std::string &name, int indent_level) {
  std::string xml;
  google_sky::StringPrintf(&xml, STR_BUFSIZE, "<%s>%.14f</%s>\n", name.c_str(),
                           kml_field.get(), name.c_str());
  Indent(&xml, indent_level);
  return xml;
}

std::string CreateKml(const google_sky::KmlField<std::string> &kml_field,
                      const std::string &name,
                      int indent_level) {
  std::string xml;
  google_sky::StringPrintf(&xml, STR_BUFSIZE, "<%s>%s</%s>\n", name.c_str(),
                           kml_field.get().c_str(), name.c_str());
  Indent(&xml, indent_level);
  return xml;
}

}  // end anonymous namespace

namespace google_sky {

// KmlIcon methods.
std::string KmlIcon::ToString(int indent_level) const {
  assert(href.has_value());
  std::string xml;
  AppendTag(&xml, "<Icon>", indent_level);
  xml.append(CreateKml(href, "href", indent_level + 1));
  AppendTag(&xml, "</Icon>", indent_level);
  return xml;
}

// KmlLatLonBox methods.
std::string KmlLatLonBox::ToString(int indent_level) const {
  assert(north.has_value());
  assert(south.has_value());
  assert(east.has_value());
  assert(west.has_value());
  std::string xml;
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
std::string KmlLatLonAltBox::ToString(int indent_level) const {
  assert(north.has_value());
  assert(south.has_value());
  assert(east.has_value());
  assert(west.has_value());
  std::string xml;
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
std::string KmlLookAt::ToString(int indent_level) const {
  assert(longitude.has_value());
  assert(latitude.has_value());
  assert(range.has_value());
  std::string xml;
  AppendTag(&xml, "<LookAt>", indent_level);
  xml.append(CreateKml(longitude, "longitude", indent_level + 1));
  xml.append(CreateKml(latitude, "latitude", indent_level + 1));
  xml.append(CreateKml(range, "range", indent_level + 1));
  AppendTag(&xml, "</LookAt>", indent_level);
  return xml;
}

// KmlLod methods.
std::string KmlLod::ToString(int indent_level) const {
  assert(min_lod_pixels.has_value());
  assert(max_lod_pixels.has_value());
  std::string xml;
  AppendTag(&xml, "<Lod>", indent_level);
  xml.append(CreateKml(min_lod_pixels, "minLodPixels", indent_level + 1));
  xml.append(CreateKml(max_lod_pixels, "maxLodPixels", indent_level + 1));
  AppendTag(&xml, "</Lod>", indent_level);
  return xml;
}

// KmlPoint methods.
std::string KmlPoint::ToString(int indent_level) const {
  assert(longitude.has_value());
  assert(latitude.has_value());
  // This is a special case CreateKml() can't handle.
  std::string coordinates;
  StringPrintf(&coordinates, STR_BUFSIZE,
               "<coordinates>%.14f,%.14f</coordinates>\n", longitude.get(),
               latitude.get());
  Indent(&coordinates, indent_level + 1);
  std::string xml;
  AppendTag(&xml, "<Point>", indent_level);
  xml.append(coordinates);
  AppendTag(&xml, "</Point>", indent_level);
  return xml;
}

// KmlLineString methods.
std::string KmlLineString::ToString(int indent_level) const {
  assert(longitudes_.size() > 0 && latitudes_.size() > 0);
  std::string xml;
  AppendTag(&xml, "<LineString>", indent_level);
  AppendTag(&xml, "<coordinates>", indent_level + 1);
  std::string line;
  for (int i = 0; i < static_cast<int>(longitudes_.size()); ++i) {
    StringPrintf(&line, STR_BUFSIZE, "%.14f,%.14f,%.14f\n", longitudes_[i],
                 latitudes_[i], 0.0);
    Indent(&line, indent_level + 2);
    xml.append(line);
  }
  AppendTag(&xml, "</coordinates>", indent_level + 1);
  AppendTag(&xml, "</LineString>", indent_level);
  return xml;
}

// KmlGroundOverlay methods.
std::string KmlGroundOverlay::ToString(int indent_level) const {
  assert(lat_lon_box.has_value());
  assert(icon.has_value());
  std::string xml;
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
  assert(ra_center >= 0.0);
  assert(ra_center < 360.0);
  
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
std::string KmlRegion::ToString(int indent_level) const {
  assert(lat_lon_alt_box.has_value());
  std::string xml;
  AppendTag(&xml, "<Region>", indent_level);
  xml.append(lat_lon_alt_box.get().ToString(indent_level + 1));
  if (lod.has_value()) {
    xml.append(lod.get().ToString(indent_level + 1));
  }
  AppendTag(&xml, "</Region>", indent_level);
  return xml;
}

// KmlLink methods.
std::string KmlLink::ToString(int indent_level) const {
  assert(href.has_value());
  std::string xml;
  AppendTag(&xml, "<Link>", indent_level);
  xml.append(CreateKml(href, "href", indent_level + 1));
  AppendTag(&xml, "</Link>", indent_level);
  return xml;
}

// KmlNetworkLink methods.
std::string KmlNetworkLink::ToString(int indent_level) const {
  assert(link.has_value());
  std::string xml;
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
std::string KmlPlacemark::ToString(int indent_level) const {
  assert(name.has_value() || description.has_value() ||
         point.has_value() || line_string.has_value());
  std::string xml;
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
std::string Kml::ToString(void) const {
  std::string xml;
  StringAppendF(&xml, STR_BUFSIZE,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  StringAppendF(&xml, STR_BUFSIZE,
                "<kml xmlns=\"http://earth.google.com/kml/2.2\" "
                "hint=\"target=sky\">\n");
  StringAppendF(&xml, STR_BUFSIZE, "<Document>\n");

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
  StringAppendF(&xml, STR_BUFSIZE, "</Document>\n");
  StringAppendF(&xml, STR_BUFSIZE, "</kml>\n");
  return xml;
}

}  // end namespace google_sky
