function addObject() {
    $.ajax({
        url: '/objects',
        type: 'POST',
        contentType: 'application/json',
        data: JSON.stringify({
            displayName: $('#addDisplayName').val(),
            classId: $('#addClassId').val()
        }),
        success: function(data) {
            alert('Object added: ' + JSON.stringify(data));
            $('#addDisplayName').val('');
            $('#addClassId').val('');
            listObjects();
        },
        error: function(response) {
            alert('Error: ' + response.responseText);
        }
    });
}

function deleteObject() {
    var objectId = $('#deleteObjectId').val();
    if (!objectId) {
        alert('Please select an object to delete.');
        return;
    }
    $.ajax({
        url: '/objects/' + objectId,
        type: 'DELETE',
        success: function(result) {
            alert('Object deleted');
            listObjects(); // Refresh the object list and dropdowns
        },
        error: function(response) {
            alert('Error: ' + response.responseText);
        }
    });
}

function editObject() {
    var objectId = $('#editObjectId').val();
    if (!objectId) {
        alert('Please select an object to edit.');
        return;
    }
    $.ajax({
        url: '/objects/' + objectId,
        type: 'PUT',
        contentType: 'application/json',
        data: JSON.stringify({
            displayName: $('#editDisplayName').val(),
            classId: $('#editClassId').val()
        }),
        success: function(result) {
            alert('Object edited: ' + JSON.stringify(result));
            $('#editDisplayName').val('');
            $('#editClassId').val('');
            listObjects(); // Refresh the object list and dropdowns.
        },
        error: function(response) {
            alert('Error: ' + response.responseText);
        }
    });
}

function uploadObjects() {
    var formData = new FormData();
    var fileInput = document.getElementById('fileUpload');
    if(fileInput.files.length === 0) {
        alert('Please select a file to upload.');
        return;
    }
    formData.append('file', fileInput.files[0]);

    $.ajax({
        url: '/upload-objects',
        type: 'POST',
        data: formData,
        contentType: false,
        processData: false,
        success: function(response) {
            alert('Objects uploaded successfully');
            listObjects(); // Refresh the objects list and dropdowns
        },
        error: function(response) {
            console.log('Error uploading objects: ' + response.responseText);
            alert('Error uploading objects: ' + response.responseText);
        }
    });
}

function generateOrder() {
    $.ajax({
        url: '/orders',
        type: 'GET',
        data: {
            complex: $('#generateComplex').is(':checked'),
            max: $('#generateMax').val()
        },
        success: function(data) {
            alert('Order generated: ' + JSON.stringify(data));
            listOrders();
        },
        error: function(response) {
            alert('Error: ' + response.responseText);
        }
    });
}

function fulfillOrder() {
    var orderId = $('#fulfillOrderId').val();
    if (!orderId) {
        alert('Please select an order to fulfill.');
        return;
    }
    $.ajax({
        url: '/orders/' + orderId + '/fulfill',
        type: 'POST',
        success: function(result) {
            alert('Order fulfilled');
            listOrders(); // Refresh the orders list and dropdowns
        },
        error: function(response) {
            alert('Error: ' + response.responseText);
        }
    });
}

function deleteOrder() {
    var orderId = $('#deleteOrderId').val();
    if (!orderId) {
        alert('Please select an order to delete.');
        return;
    }
    $.ajax({
        url: '/orders/' + orderId,
        type: 'DELETE',
        success: function(result) {
            alert('Order deleted successfully');
            listOrders(); // Refresh the orders list and dropdowns
        },
        error: function(response) {
            alert('Error: ' + response.responseText);
        }
    });
}

function listObjects() {
    $.get('/objects', function(data) {
        var objectsHtml = data.map(function(obj) {
            return '<p>ID: ' + obj.id + ', DisplayName: ' + obj.displayName + ', ClassId: ' + obj.classId + '</p>';
        }).join('');
        var objectsOptions = '<option value="">Select Object</option>';
        data.forEach(function(obj) {
            // This line concatenates the display name and ID for the dropdown
            objectsOptions += '<option value="' + obj.id + '">' + obj.displayName + ' (' + obj.id + ')</option>';
        });
        $('#objectsContainer').html(objectsHtml);
        $('#deleteObjectId').html(objectsOptions);
        $('#editObjectId').html(objectsOptions);
    });
}

function listOrders() {
    $.get('/orders/list', function(data) {
        var ordersHtml = '';
        var ordersOptions = '<option value="">Select Order</option>';
        data.forEach(function(order) {
            var orderObjects = JSON.parse(order.objects); // Parse the JSON string of objects
            var objectsHtml = '[';
            orderObjects.forEach(function(obj) {
                var displayName = '';
                $.ajax({
                    url: '/objects/' + obj.id,
                    type: 'GET',
                    async: false,
                    success: function(object) {
                        objectsHtml += object.displayName + ',';
                    }
                });
            });
            objectsHtml = objectsHtml.slice(0, -1) + ']';
            var fulfilledClass = order.fulfilled ? 'fulfilled' : 'unfulfilled';
            ordersHtml += '<p>ID: ' + order.id + ', Objects: ' + objectsHtml + ', Timestamp: ' + order.timestamp + ', Fulfilled: <span class="' + fulfilledClass + '">' + order.fulfilled + '</span></p>'

            ordersOptions += '<option value="' + order.id + '">' + order.id + ' (Timestamp: ' + order.timestamp + ')</option>';
        });
        $('#ordersContainer').html(ordersHtml);
        $('#deleteOrderId').html(ordersOptions);
        $('#fulfillOrderId').html(ordersOptions);
        $('#editOrderId').html(ordersOptions);
    });
}

// Call the listing functions on page load
$(document).ready(function() {
    listObjects();
    listOrders();
    setInterval(listOrders, 3000);
});