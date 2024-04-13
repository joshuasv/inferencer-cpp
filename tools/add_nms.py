from IPython import embed
import onnx
import onnxruntime as ort
import numpy as np
import cv2

INT_MAX = 2147483647

model_fpath = "../models/best.onnx"
out_fpath = "../models/best_nms.onnx"

model = onnx.load_model(model_fpath)
graph = model.graph

# Remove model.22/Concat_5
model.graph.node.remove(graph.node[-1])

# Remove original output0
model.graph.output.remove(graph.output[0])

# Create constant tensors for NMS input
max_output_boxes_per_class = onnx.helper.make_tensor("max_output_boxes_per_class", onnx.TensorProto.INT64, [1], [200])
iou_threshold = onnx.helper.make_tensor("iou_threshold", onnx.TensorProto.FLOAT, [1], [0.5])
score_threshold = onnx.helper.make_tensor("score_threshold", onnx.TensorProto.FLOAT, [1], [0.45])
graph.initializer.append(max_output_boxes_per_class)
graph.initializer.append(iou_threshold)
graph.initializer.append(score_threshold)

# Transpose the bboxes node (see boxes attribute in NonMaxSupression)
graph.node.append(
    onnx.helper.make_node("Transpose", inputs=["/model.22/Mul_2_output_0"], outputs=["boxes"], perm=(0,2,1))
)

# Add NMS, https://onnx.ai/onnx/operators/onnx__NonMaxSuppression.html
graph.node.append(
    onnx.helper.make_node(
        "NonMaxSuppression",
        inputs=["boxes", "/model.22/Sigmoid_output_0", "max_output_boxes_per_class", "iou_threshold", "score_threshold"],
        outputs=["nms_output"],
        center_point_box=1, # box data is supplied as [x_center, y_center, w, h]
    )
)

# Extract classes from NMS output
classes_indices = onnx.helper.make_tensor("classes_indices", onnx.TensorProto.INT32, [1], [1])
graph.initializer.append(classes_indices)
graph.node.append(
    onnx.helper.make_node(
        "Gather",
        inputs=["nms_output", "classes_indices"],
        outputs=["detected_classes"],
        axis=1
    )
)

# Extract indices from NMS output so we can index boxes
nms_indices = onnx.helper.make_tensor("nms_indices", onnx.TensorProto.INT32, [1], [2])
graph.initializer.append(nms_indices)
graph.node.append(
    onnx.helper.make_node(
        "Gather",
        inputs=["nms_output", "nms_indices"],
        outputs=["indices"],
        axis=1
    )
)

# Squeeze last dimension of indices from NMS
squeeze_axes = onnx.helper.make_tensor("squeeze_axes", onnx.TensorProto.INT64, [1], [-1])
graph.initializer.append(squeeze_axes)
graph.node.append(
    onnx.helper.make_node(
        "Squeeze",
        inputs=["indices", "squeeze_axes"],
        outputs=["indices_vec"],
    )
)

# Extract detected boxes
graph.node.append(
    onnx.helper.make_node(
        "Gather",
        inputs=["boxes", "indices_vec"],
        outputs=["detected_boxes"],
        axis=1
    )
)

# Extract detected scores using the whole result of the NMS
graph.node.append(
    onnx.helper.make_node(
        "GatherND",
        inputs=["/model.22/Sigmoid_output_0", "nms_output"],
        outputs=["detected_scores"],
        batch_dims=0
    )
)

# Squeeze classes last dimension
graph.node.append(
    onnx.helper.make_node(
        "Squeeze",
        inputs=["detected_classes", "squeeze_axes"],
        outputs=["detected_classes_vec"],
    )
)

# Squeeze boxes batch dimension
squeeze_batch_axes = onnx.helper.make_tensor("squeeze_batch_axes", onnx.TensorProto.INT64, [1], [0])
graph.initializer.append(squeeze_batch_axes)
graph.node.append(
    onnx.helper.make_node(
        "Squeeze",
        inputs=["detected_boxes", "squeeze_batch_axes"],
        outputs=["detected_boxes_no_batch"],
    )
)

# Add to output raw boxes and scores
graph.output.append(
    onnx.helper.make_tensor_value_info("detected_classes_vec", onnx.TensorProto.INT64, shape=["num_results"])
)

graph.output.append(
    onnx.helper.make_tensor_value_info("detected_scores", onnx.TensorProto.FLOAT, shape=["num_results"])
)

graph.output.append(
    onnx.helper.make_tensor_value_info("detected_boxes_no_batch", onnx.TensorProto.FLOAT, shape=["num_results", 4])
)
# TODO; remove
# graph.output.append(
#     onnx.helper.make_tensor_value_info("nms_output", onnx.TensorProto.INT64, shape=["num_results", 3])
# )

# Check and simplify final model
onnx.checker.check_model(model)
# Save to disk
onnx.save(model, out_fpath)


# Test inference
session = ort.InferenceSession(out_fpath)
input_name = session.get_inputs()[0].name
output_names = [output.name for output in session.get_outputs()]
input_fname = "./test3.png"
pred_fname = input_fname.replace(".png", "-pred.png")
img_or = cv2.imread(input_fname)
img = img_or.copy().astype(np.float32)
img *= (1 / 255.)
img = cv2.resize(img, (640, 640))
img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
data = np.moveaxis(img, -1, 0)
data = data[None]

factor_x = 1.
factor_y = 0.75
classes = ["white", "blue", "orange"]
colors = [(255, 255, 255), (165, 0, 0), (0, 165, 255)]
outputs = session.run(output_names, {input_name: data})
classes = outputs[0]
scores = outputs[1]
boxes = outputs[2]
print(f"{len(outputs)=}")
print(f"classes: {classes.shape=} {classes}")
# print(outputs[3])
# print(f"scores: {outputs[1].shape=}")
# print(f"boxes: {outputs[2].shape=}")


# print(boxes)
# boxes[:,0] = (boxes[:,0] - 0.5 * boxes[:,2]) * factor_x
# boxes[:,1] = (boxes[:,1] - 0.5 * boxes[:,3]) * factor_y
# boxes[:,2] = (boxes[:,2] + boxes[:,0]) * factor_x
# boxes[:,3] = (boxes[:,3] + boxes[:,1]) * factor_y
# # boxes[:,2] += boxes[:,0]
# # boxes[:,3] += boxes[:,1]
# boxes = np.round(boxes).astype(np.int32)

# for i, box in enumerate(boxes):
#   class_str = f"{classes[i]} {scores[i]*100:.2f}"
#   retval, baseline = cv2.getTextSize(class_str, cv2.FONT_HERSHEY_DUPLEX, 1, 2)
#   cv2.rectangle(img_or, box[:2], box[2:], color=colors[i])
#   cv2.rectangle(img_or, (box[0], box[1] - 40), (retval[0] + 10, retval[1] + 20), colors[i], cv2.FILLED)
#   cv2.putText(img_or, class_str, (box[0] + 5, box[1] - 10), cv2.FONT_HERSHEY_DUPLEX, 1, (0, 0, 0), 2, 0)
# cv2.imwrite(pred_fname, img_or)

