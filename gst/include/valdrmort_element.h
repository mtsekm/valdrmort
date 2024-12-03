#ifndef VALDRMORT_ELEMENT_H
#define VALDRMORT_ELEMENT_H

#include "DRMBackend.h"

#include <gst/base/gstbasetransform.h>

#include <memory>

G_BEGIN_DECLS

#define VALDRMORT_TYPE_ELEMENT (valdrmort_element_get_type())
#define VALDRMORT_ELEMENT(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), VALDRMORT_TYPE_ELEMENT, valdrmortElement))
#define VALDRMORT_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), VALDRMORT_TYPE_ELEMENT, valdrmortElementClass))
#define IS_VALDRMORT_ELEMENT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), VALDRMORT_TYPE_ELEMENT))
#define IS_VALDRMORT_ELEMENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), VALDRMORT_TYPE_ELEMENT))

typedef struct _valdrmortElement valdrmortElement;
typedef struct _valdrmortElementClass valdrmortElementClass;

struct _valdrmortElement {
    GstBaseTransform parent;
    std::unique_ptr<DRMBackend> drm_backend; // DRM backend instance
    gboolean initialized;                    // Whether the element is initialized
};

struct _valdrmortElementClass {
    GstBaseTransformClass parent_class;
};

GType valdrmort_element_get_type(void);

G_END_DECLS

#endif // VALDRMORT_ELEMENT_H
