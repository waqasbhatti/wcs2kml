// Copyright (c) 2007-2009, Google Inc.
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

// Classes for representing the subset of KML used by Sky
//
// This collection of classes is by no means a complete description of the KML
// specification.  It was designed to add some ease of use and structure to
// the output of the kinds of KML produced by the Sky related programs, which
// is very small (currently there are 3 basic kinds of KML documents).
// An additional consideration was that other KML code inside Google could not
// be open sourced.
//
// This code was written mostly referring to the KML 2.1 reference with a
// few minor additions from 2.2 for Sky.  See the KML 2.1 reference at
// http://www.corp.google.com/~robinz/KML21/kml_tags_21.html for more
// information.
//
// The basic idea of this code is that there is a class for each non-trivial
// element.  Each element class implements a ToString(indent_level) method that
// returns the XML for that element indented to the proper level.
//
// Because part of the reason for developing this class was to enforce the
// KML schema at a code level, any required members of the various element
// classes that are not present on calls to ToString() will cause the method to
// die (and there are almost always required members).  See the documentation
// for each element class for a list of required members.
//
// Each child element is represented as a public instance of a template class
// to reduce the amount of boilerplate code.  The template class, KmlField,
// handles all of the accessor methods, which are complicated by the fact
// that some fields are required and some are optional.  See the example code
// below for concrete examples of how to access child element.
//
// The exception is that some classes (KML and KmlLineString) can contain more
// than one child element of a given type or more than one piece of data for a
// given element.  These classes hold vectors of data rather than KmlFields and
// provide methods Add* and Clear* for adding and removing elements.  Another
// oddity comes from classes that contain the <coodinates> element (KmlPoint
// and KmlLineString).  Instead of storing a KmlField<KmlCoordinates> object,
// they store the components separately, i.e. latitude and longitude.  Aside
// from these exceptions, the child elements should be named in sensible way
// that directly mirrors the KML schema.
//
// The element classes KmlLatLonBox and KmlLatLonAltBox provide a convenience
// method FromBoundingBox() for creating them from a BoundingBox object.
//
// Example Usage:
//
// // We're going to create a KML document containing a single Ground Overlay.
//
// // Here, href is a public instance of KmlField<string>.  KmlField supports
// // get(), set(), clear(), and has_value().  This syntax may appear a bit
// // odd, but it saves many lines of boilerplate accessor code.
// KmlIcon icon;
// icon.href.set("foo.png");
//
// // Here's how to get a copy of the href value.
// std::string href_copy = icon.href.get();
//
// KmlLatLonBox lat_lon_box;
// lat_lon_box.north.set(12.0);
// lat_lon_box.south.set(11.0);
// lat_lon_box.east.set(32.0);
// lat_lon_box.west.set(30.0);
//
// KmlLookAt look_at;
// look_at.latitude.set(11.5);
// look_at.longitude.set(31.0);
// look_at.range.set(10.0);
//
// KmlGroundOverlay ground_overlay;
// ground_overlay.name.set("A sample overlay");
// ground_overlay.icon.set(icon);
// ground_overlay.lat_lon_box.set(lat_lon_box);
// ground_overlay.look_at.set(look_at);
//
// KML kml;
// kml.AddGroundOverlay(ground_overlay);
//
// // Get the raw XML string describing the KML.
// std::string kml_string = kml.ToString();

#ifndef KML_H__
#define KML_H__

#include <cassert>
#include <string>
#include <vector>

namespace google_sky {

// Forward declaration.
class BoundingBox;

// Template class for representing KML fields.
template<class T>
class KmlField {
 public:
  // Creates a KmlField with no value.  The set() method must be called to
  // associate a value with this field.
  KmlField() : value_() {
    has_value_ = false;
  }

  // NB: Compiler generated copy functions are fine.
 
 // Sets the value.  The only way to indicate that a KmlField contains a
 // value is to set the value.
  void set(const T &value) {
    value_ = value;
    has_value_ = true;
  }

  // Removes the value present.
  void clear() {
    has_value_ = false;
  }

  // Returns the value.
  T get() const {
    assert(has_value_);
    return value_;
  }
  
  // Returns whether this field has an associated value.  After construction,
  // no KmlField has an associated value.
  bool has_value() const {
    return has_value_;
  }

 private:
  bool has_value_;
  T value_;
};

// <Icon> class
// Required fields: href
class KmlIcon {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<std::string> href;
};

// <LatLonBox> class
// Required fields: north, south, east, west
class KmlLatLonBox {
 public:
  virtual ~KmlLatLonBox() {
    // Nothing needed (just here to keep compiler happy).
  }
  
  // NB: Compiler generated ctor and copy functions are fine.
  virtual std::string ToString(int indent_level) const;
  KmlField<double> north;
  KmlField<double> south;
  KmlField<double> east;
  KmlField<double> west;
  KmlField<double> rotation;
};

// <LatLonAltBox> class
// Required fields: north, south, east, west
class KmlLatLonAltBox : public KmlLatLonBox {
 public:
  virtual ~KmlLatLonAltBox() {
    // Nothing needed (just here to keep compiler happy).
  }
  
  // NB: Compiler generated ctor and copy functions are fine.
  virtual std::string ToString(int indent_level) const;
  KmlField<double> min_altitude;
  KmlField<double> max_altitude;
};

// <LookAt> class
// Required fields: longitude, latitude, range
class KmlLookAt {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<double> longitude;
  KmlField<double> latitude;
  KmlField<double> range;
};

// <Lod> class
// Required fields: min_lod_pixels, max_lod_pixels
class KmlLod {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<int> min_lod_pixels;
  KmlField<int> max_lod_pixels;
};

// <Point> class
// Required fields: longitude, latitude
class KmlPoint {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<double> longitude;
  KmlField<double> latitude;
};

// <LineString> class
// This class is a bit odd because the coordinates tag can contain multiple
// tuples of values.
// Required fields: one value for longitude, latitude
class KmlLineString {
 public:
  KmlLineString() : longitudes_(), latitudes_() {
    // Nothing needed.
  }
  
  ~KmlLineString() {
    // Nothing needed.
  }

  // NB: Compiler generated copy methods are fine.

  std::string ToString(int indent_level) const;

  inline void AddCoordinate(double longitude, double latitude) {
    longitudes_.push_back(longitude);
    latitudes_.push_back(latitude);
  }
  
  inline void ClearCoordinates(void) {
    longitudes_.clear();
    latitudes_.clear();
  }

 private:
  std::vector<double> longitudes_;
  std::vector<double> latitudes_;
};

// <GroundOverlay> class
// Required fields: lat_lon_box, icon
class KmlGroundOverlay {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  
  // Populates the dimensions of the LatLonBox at LookAt from a BoundingBox
  // object describing boundaries of an image in ra, dec space.
  void FromBoundingBox(const BoundingBox &bounding_box);
  
  KmlField<std::string> name;
  KmlField<int> draw_order;
  KmlField<KmlIcon> icon;
  KmlField<KmlLatLonBox> lat_lon_box;
  KmlField<KmlLookAt> look_at;
};

// <Region> class
// Required fields: lat_lon_alt_box
class KmlRegion {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<KmlLatLonAltBox> lat_lon_alt_box;
  KmlField<KmlLod> lod;
};

// <Link> class
// Required fields: href
class KmlLink {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<std::string> href;
};

// <NetworkLink> class
// Required fields: link
class KmlNetworkLink {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<std::string> name;
  KmlField<KmlRegion> region;
  KmlField<KmlLink> link;
};

// <Placemark> class
// Required fields: at least one of name, description, point, or line_string
//                  must be present
class KmlPlacemark {
 public:
  // NB: Compiler generated ctor, dtor, and copy functions are fine.
  std::string ToString(int indent_level) const;
  KmlField<std::string> name;
  KmlField<std::string> description;
  KmlField<KmlLookAt> look_at;
  KmlField<KmlPoint> point;
  KmlField<KmlLineString> line_string;
};

// <kml> class
// See the documentation at the top of this file.
// Required fields: none (but the KML is not very interesting then)
class Kml {
 public:
  // Creates a KML file with Document as the root Feature.  The GroundOverlays,
  // Placemarks and NetworkLinks are added as children of the Document.  Note
  // that there is one Region defined for the Document which cascades to all
  // children.  NetworkLinks should therefore define their own Region.
  Kml() : region(), ground_overlays_(), placemarks_(), network_links_() {
    // Nothing needed.
  }

  ~Kml() {
    // Nothing needed.
  }

  // NB: Compiler generated copy methods are fine.

  // Returns a human readable std::string representation of the KML object.
  // This is the method to call to generate the raw XML string.
  std::string ToString(void) const;
  
  inline void AddGroundOverlay(const KmlGroundOverlay &ground_overlay) {
    ground_overlays_.push_back(ground_overlay);
  }

  inline void ClearGroundOverlays(void) {
    ground_overlays_.clear();
  }
  
  inline void AddPlacemark(const KmlPlacemark &placemark) {
    placemarks_.push_back(placemark);
  }

  inline void ClearPlacemarks(void) {
    placemarks_.clear();
  }
   
  inline void AddNetworkLink(const KmlNetworkLink &network_link) {
    network_links_.push_back(network_link);
  }
  
  inline void ClearNetworkLinks(void) {
    network_links_.clear();
  }
  
  KmlField<KmlRegion> region;
  
 private:
  std::vector<KmlGroundOverlay> ground_overlays_;
  std::vector<KmlPlacemark> placemarks_;
  std::vector<KmlNetworkLink> network_links_;
  
  // Don't allow copying.
  Kml(const Kml &);
  Kml &operator=(const Kml &);
};  // end Kml

}  // end namespace google_sky

#endif  // KML_H__
