/**
 * SECTION:element-formulaoneparse
 *
 * FIXME:Describe formulaoneparse here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! formulaoneparse ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>
#include <gst/gst.h>

#include "gstformulaoneparse.h"

GST_DEBUG_CATEGORY_STATIC(gst_formula_one_parse_debug);
#define GST_CAT_DEFAULT gst_formula_one_parse_debug
#define SRC_CAPS "video/x-raw, format=(string) RGB, width=(int) 300, height=(int) 300"

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS("ANY"));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
                                                                  GST_PAD_SRC,
                                                                  GST_PAD_ALWAYS,
                                                                  GST_STATIC_CAPS(SRC_CAPS));

#define gst_formula_one_parse_parent_class parent_class
G_DEFINE_TYPE(GstFormulaOneParse, gst_formula_one_parse, GST_TYPE_ELEMENT);

static void gst_formula_one_parse_set_property(GObject *object, guint prop_id,
                                               const GValue *value, GParamSpec *pspec);
static void gst_formula_one_parse_get_property(GObject *object, guint prop_id,
                                               GValue *value, GParamSpec *pspec);

static gboolean gst_formula_one_parse_sink_event(GstPad *pad, GstObject *parent, GstEvent *event);
static GstFlowReturn gst_formula_one_parse_chain(GstPad *pad, GstObject *parent, GstBuffer *buf);

/* GObject vmethod implementations */

/* initialize the formulaoneparse's class */
static void
gst_formula_one_parse_class_init(GstFormulaOneParseClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *)klass;
  gstelement_class = (GstElementClass *)klass;

  gobject_class->set_property = gst_formula_one_parse_set_property;
  gobject_class->get_property = gst_formula_one_parse_get_property;

  g_object_class_install_property(gobject_class, PROP_SILENT,
                                  g_param_spec_boolean("silent", "Silent", "Produce verbose output ?",
                                                       FALSE, G_PARAM_READWRITE));

  gst_element_class_set_details_simple(gstelement_class,
                                       "FormulaOneParse",
                                       "Parser",
                                       "Parses F1 2018 Telemetry Data",
                                       "Sean <<user@hostname.org>>");

  gst_element_class_add_pad_template(gstelement_class,
                                     gst_static_pad_template_get(&src_factory));
  gst_element_class_add_pad_template(gstelement_class,
                                     gst_static_pad_template_get(&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_formula_one_parse_init(GstFormulaOneParse *filter)
{
  filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
  gst_pad_set_event_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_formula_one_parse_sink_event));
  gst_pad_set_chain_function(filter->sinkpad,
                             GST_DEBUG_FUNCPTR(gst_formula_one_parse_chain));
  GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS(filter->srcpad);
  gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

  filter->silent = FALSE;
}

static void
gst_formula_one_parse_set_property(GObject *object, guint prop_id,
                                   const GValue *value, GParamSpec *pspec)
{
  GstFormulaOneParse *filter = GST_FORMULAONEPARSE(object);

  switch (prop_id)
  {
  case PROP_SILENT:
    filter->silent = g_value_get_boolean(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

static void
gst_formula_one_parse_get_property(GObject *object, guint prop_id,
                                   GValue *value, GParamSpec *pspec)
{
  GstFormulaOneParse *filter = GST_FORMULAONEPARSE(object);

  switch (prop_id)
  {
  case PROP_SILENT:
    g_value_set_boolean(value, filter->silent);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_formula_one_parse_sink_event(GstPad *pad, GstObject *parent, GstEvent *event)
{
  GstFormulaOneParse *filter;
  gboolean ret;

  filter = GST_FORMULAONEPARSE(parent);

  GST_LOG_OBJECT(filter, "Received %s event: %" GST_PTR_FORMAT,
                 GST_EVENT_TYPE_NAME(event), event);

  switch (GST_EVENT_TYPE(event))
  {
  case GST_EVENT_CAPS:
  {
    GstCaps *caps;

    gst_event_parse_caps(event, &caps);
    /* do something with the caps */

    /* and forward */
    ret = gst_pad_event_default(pad, parent, event);
    break;
  }
  default:
    ret = gst_pad_event_default(pad, parent, event);
    break;
  }
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_formula_one_parse_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
  GstVideoFrame vframe;
  GstVideoInfo video_info;
  GstBuffer *video_buffer;
  GstMemory *memory;
  gint size;

  GstFormulaOneParse *filter;
  filter = GST_FORMULAONEPARSE(parent);

  if (filter->silent == FALSE)
  {
    GstMapInfo map;
    gst_buffer_map(buf, &map, GST_MAP_READ);
    PacketHeader *ph = (PacketHeader *)map.data;
    g_print("UDP Packet Identifier - %d\n", ph->m_packetId);
    if (ph->m_packetId == 6)
    {
      PacketCarTelemetryData *carTelemetry = (PacketCarTelemetryData *)map.data;
      // g_print("PacketCarTelemetryData Size - %u\n", sizeof(PacketCarTelemetryData));
      g_print("Car Index - %u\n", ph->m_playerCarIndex);
      CarTelemetryData playerCar = carTelemetry->m_carTelemetryData[ph->m_playerCarIndex];
      g_print("Speed - %u\n", playerCar.m_speed);
      g_print("Gear - %i\n", playerCar.m_gear);
      g_print("Engine RPM - %u\n", playerCar.m_engineRPM);

      size = 300 * 300 * 3;
      video_buffer = gst_buffer_new();
      gst_video_info_from_caps(&video_info, gst_caps_from_string(SRC_CAPS));
      memory = gst_allocator_alloc(NULL, size, NULL);
      gst_buffer_insert_memory(video_buffer, -1, memory);
      // set RGB pixels to black one at a time
      if (gst_video_frame_map(&vframe, &video_info, video_buffer, GST_MAP_WRITE))
      {
        guint8 *pixels = GST_VIDEO_FRAME_PLANE_DATA(&vframe, 0);
        guint stride = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe, 0);
        guint pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe, 0);
        int h, w, height = 300, width = 300;
        for (h = 0; h < height; ++h)
        {
          for (w = 0; w < width; ++w)
          {
            guint8 *pixel = pixels + h * stride + w * pixel_stride;
            memset(pixel, 0, pixel_stride);
          }
        }
        gst_video_frame_unmap(&vframe);
      }
      gst_buffer_unmap(buf, &map);
      /* push out the buffer */
      return gst_pad_push(filter->srcpad, video_buffer);
    }
  }
  return GST_FLOW_OK;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
formulaoneparse_init(GstPlugin *formulaoneparse)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template formulaoneparse' with your description
   */
  GST_DEBUG_CATEGORY_INIT(gst_formula_one_parse_debug, "formulaoneparse",
                          0, "Template formulaoneparse");

  return gst_element_register(formulaoneparse, "formulaoneparse", GST_RANK_NONE,
                              GST_TYPE_FORMULAONEPARSE);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "formulaoneparsepackage"
#endif

/* gstreamer looks for this structure to register formulaoneparses
 *
 * exchange the string 'Template formulaoneparse' with your formulaoneparse description
 */
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    formulaoneparse,
    "Template formulaoneparse",
    formulaoneparse_init,
    "0.0.1",
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/")
