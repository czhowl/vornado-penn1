/*
 * WebGL Water
 * http://madebyevan.com/webgl-water/
 *
 * Copyright 2011 Evan Wallace
 * Released under the MIT license
 */

function text2html(text) {
  return text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/\n/g, '<br>');
}

function handleError(text) {
  var html = text2html(text);
  if (html == 'WebGL not supported') {
    html = 'Your browser does not support WebGL.<br>Please see\
    <a href="http://www.khronos.org/webgl/wiki/Getting_a_WebGL_Implementation">\
    Getting a WebGL Implementation</a>.';
  }
  var loading = document.getElementById('loading');
  loading.innerHTML = html;
  loading.style.zIndex = 1;
}

window.onerror = handleError;

var gl = GL.create();
var water;
var cubemap;
var renderer;
var angleX = -90;
var angleY = -0;

// Sphere physics info
var useSpherePhysics = false;
var center;
var oldCenter;
var velocity;
var gravity;
var radius;
var size = 0.01;
var speed = 2;
var paused = false;
var r = 0.5;
var g = 0.5;
var b = 0.6;
var sr = 10;
var sg = 5;
var sb = 0;
var sunsize = 500.0;
var strength = 0.01;

window.onload = function() {
  var ratio = window.devicePixelRatio || 1;
  var help = document.getElementById('help');

  function onresize() {
    var width = innerWidth - help.clientWidth - 20;
    var height = innerHeight;
    gl.canvas.width = width * ratio;
    gl.canvas.height = height * ratio;
    gl.canvas.style.width = width + 'px';
    gl.canvas.style.height = height + 'px';
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
    gl.matrixMode(gl.PROJECTION);
    gl.loadIdentity();
    gl.perspective(45, gl.canvas.width / gl.canvas.height, 0.01, 100);
    gl.matrixMode(gl.MODELVIEW);
    draw();
  }

  document.body.appendChild(gl.canvas);
  gl.clearColor(0, 0, 0, 1);

  water = new Water();
  renderer = new Renderer();
  cubemap = new Cubemap({
    xneg: document.getElementById('xneg'),
    xpos: document.getElementById('xpos'),
    yneg: document.getElementById('ypos'),
    ypos: document.getElementById('ypos'),
    zneg: document.getElementById('zneg'),
    zpos: document.getElementById('zpos')
  });

  if (!water.textureA.canDrawTo() || !water.textureB.canDrawTo()) {
    throw new Error('Rendering to floating-point textures is required but not supported');
  }

  center = oldCenter = new GL.Vector(-0.4, -0.75, 0.2);
  velocity = new GL.Vector();
  gravity = new GL.Vector(0, -4, 0);
  radius = 0.25;

  for (var i = 0; i < 20; i++) {
    water.addDrop(Math.random() * 2 - 1, Math.random() * 2 - 1, size, (i & 1) ? 0.01 : -0.01);
  }

  document.getElementById('loading').innerHTML = '';
  onresize();

  var requestAnimationFrame =
    window.requestAnimationFrame ||
    window.webkitRequestAnimationFrame ||
    function(callback) { setTimeout(callback, 0); };

  var prevTime = new Date().getTime();
  function animate() {
    var nextTime = new Date().getTime();
    if (!paused) {
      update((nextTime - prevTime) / 1000);
      draw();
    }
    prevTime = nextTime;
    requestAnimationFrame(animate);
  }
  requestAnimationFrame(animate);

  window.onresize = onresize;

  var prevHit;
  var planeNormal;
  var mode = -1;
  var MODE_ADD_DROPS = 0;
  var MODE_ORBIT_CAMERA = 1;

  var oldX, oldY;

  function startDrag(x, y, e) {
    oldX = x;
    oldY = y;
    var tracer = new GL.Raytracer();
    var ray = tracer.getRayForPixel(x * ratio, y * ratio);
    var pointOnPlane = tracer.eye.add(ray.multiply(-tracer.eye.y / ray.y));
    var sphereHitTest = GL.Raytracer.hitTestSphere(tracer.eye, ray, center, radius);
    if (!e) {
      mode = MODE_ADD_DROPS;
      duringDrag(x, y);
    } else {
      mode = MODE_ORBIT_CAMERA;
    }
  }

  function duringDrag(x, y) {
    switch (mode) {
      case MODE_ADD_DROPS: {
        var tracer = new GL.Raytracer();
        var ray = tracer.getRayForPixel(x * ratio, y * ratio);
        var pointOnPlane = tracer.eye.add(ray.multiply(-tracer.eye.y / ray.y));
        water.addDrop(pointOnPlane.x, pointOnPlane.z, size, strength);
        if (paused) {
          water.updateNormals();
          renderer.updateCaustics(water);
        }
        break;
      }
      case MODE_ORBIT_CAMERA: {
        angleY -= x - oldX;
        angleX -= y - oldY;
        //angleX = Math.max(-89.999, Math.min(89.999, angleX));
        break;
      }
    }
    oldX = x;
    oldY = y;
    if (paused) draw();
  }

  function stopDrag() {
    mode = -1;
  }

  function isHelpElement(element) {
    return element === help || element.parentNode && isHelpElement(element.parentNode);
  }

  document.onmousedown = function(e) {
    // if (!isHelpElement(e.target)) {
      e.preventDefault();
      startDrag(e.pageX, e.pageY, isHelpElement(e.target));
    // }
  };

  document.onmousemove = function(e) {
    duringDrag(e.pageX, e.pageY);
  };

  document.onmouseup = function() {
    stopDrag();
  };

  document.ontouchstart = function(e) {
    if (e.touches.length === 1 && !isHelpElement(e.target)) {
      e.preventDefault();
      startDrag(e.touches[0].pageX, e.touches[0].pageY, e.touches.length === 1 && !isHelpElement(e.target));
    }
  };

  document.ontouchmove = function(e) {
    if (e.touches.length === 1) {
      duringDrag(e.touches[0].pageX, e.touches[0].pageY);
    }
  };

  document.ontouchend = function(e) {
    if (e.touches.length == 0) {
      stopDrag();
    }
  };

  function changewaterColor(){
    var color = document.getElementById("watercolor").value;
    
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(color);
    r = parseInt(result[1], 16) / 255.0;
    g = parseInt(result[2], 16) / 255.0;
    b = parseInt(result[3], 16) / 255.0;
    draw();
}

  document.getElementById("watercolor").addEventListener("change", changewaterColor, false);
  
  function changeSunColor(){
    var color = document.getElementById("suncolor").value;
    
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(color);
    sr = parseInt(result[1], 16) / 25.5;
    sg = parseInt(result[2], 16) / 25.5;
    sb = parseInt(result[3], 16) / 25.5;
    draw();
}

  document.getElementById("suncolor").addEventListener("change", changeSunColor, false);

  document.onkeydown = function(e) {
    if (e.which == ' '.charCodeAt(0)) paused = !paused;
    else if (e.which == 'L'.charCodeAt(0) && paused) draw();
    else if (e.which == 'Q'.charCodeAt(0) && paused) draw();
    else if (e.which == 'W'.charCodeAt(0) && paused) draw();
    else if (e.which == 'Z'.charCodeAt(0) && paused) draw();
    else if (e.which == 'X'.charCodeAt(0) && paused) draw();
    else if (e.which == 'A'.charCodeAt(0) && paused) draw();
    else if (e.which == 'S'.charCodeAt(0) && paused) draw();
  };

  var frame = 0;

  function update(seconds) {
    if (seconds > 1) return;
    frame += seconds * 2;

    // Displace water around the sphere
    oldCenter = center;

    // Update the water simulation and graphics
    for(var i = 0; i < speed; i ++){
      water.stepSimulation();
    }

    water.addDrop(Math.random() * 1.0 -0.5 , 1.0 , 0.1, 0.0002);
    water.addDrop(Math.random() * 1.0 -0.5 , -1.0 , 0.1, 0.0002);
    water.addDrop(1.0, Math.random() * 1.0 -0.5, 0.1, 0.0002);
    water.addDrop(-1.0, Math.random() * 1.0 -0.5, 0.1, 0.0002);
    //water.addDrop(Math.random() * 0.1 - 0.99,Math.random() * 0.1 - 0.99, 0.04, 0.0005);
    //water.stepSimulation();
    water.updateNormals();
    renderer.updateCaustics(water);
  }

  function draw() {
    // Change the light direction to the camera look vector when the L key is pressed
    //if (GL.keys.L) {
      renderer.lightDir = GL.Vector.fromAngles((90 - angleY) * Math.PI / 180, -angleX * Math.PI / 180);
      if (paused) renderer.updateCaustics(water);
    //}
    if (GL.keys.Q) {
      size -= 0.001;
      if(size < 0.001) size = 0.001; 
    }
    if (GL.keys.W) {
      size += 0.001;
    }
    if (GL.keys.Z) {
      speed -= 1;
      if(speed < 1) speed = 1; 
    }
    if (GL.keys.X) {
      speed += 1;
      if(speed > 20) speed = 20; 
    }

    if (GL.keys.A) {
      sunsize *= 2;
      if(sunsize > 5000000) sunsize = 500000; 
    }
    if (GL.keys.S) {
      sunsize /= 2;
      if(sunsize < 0.05) sunsize = 0.05; 
    }

    if (GL.keys.N) {
      strength -= 0.001;
      if(strength < 0.001) strength = 0.001; 
    }
    if (GL.keys.M) {
      strength += 0.05;
      if(strength >0.05) strength = 0.05; 
    }

    // angleX = -90;
    // angleY = 0;

    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.loadIdentity();
    gl.translate(0, 0, -4);
    gl.rotate(90, 1, 0, 0);
    gl.rotate(0, 0, 1, 0);
    gl.translate(0, 0.1, 0);
    

    gl.enable(gl.DEPTH_TEST);
    renderer.sphereCenter = center;
    renderer.sphereRadius = radius;
    //renderer.renderCube();
    renderer.renderWater(water, cubemap, new GL.Vector(r, g,b), new GL.Vector(sr, sg,sb), sunsize);
    //renderer.renderSphere();
    gl.disable(gl.DEPTH_TEST);
  }
};
