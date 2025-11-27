import cv2
import time
import yaml
import os
from flask import Flask, Response, render_template_string

app = Flask(__name__)

# Load Configuration from the Single Source of Truth
CONFIG_PATH = "configs/zones.yaml"

def load_config():
    # 1. Resolve absolute path
    dashboard_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(dashboard_dir)
    config_path = os.path.join(project_root, 'configs', 'zones.yaml')
    
    if not os.path.exists(config_path):
        print(f"⚠️ Config not found at {config_path}. Using defaults.")
        return {}
    
    # 2. Read file & Filter out OpenCV headers
    with open(config_path, 'r') as f:
        content = f.read()
        
        # FIX: Remove the OpenCV "%YAML:1.0" header if present
        if content.startswith("%YAML"):
            content = content.split('\n', 1)[1]
            
        print(f"✅ Loaded config from: {config_path}")
        return yaml.safe_load(content)

# HTML Template
PAGE_HTML = """
<html>
<head>
    <title>CCM EdgeVision | Live Dashboard</title>
    <style>
        body { background-color: #121212; color: #ffffff; font-family: monospace; text-align: center; }
        h1 { color: #00ff00; margin-top: 20px; }
        .container { display: flex; justify-content: center; margin-top: 20px; }
        img { border: 2px solid #333; box-shadow: 0 0 20px rgba(0, 255, 0, 0.2); }
        .stats { margin-top: 20px; color: #888; }
    </style>
</head>
<body>
    <h1>CCM EdgeVision // Remote Stream</h1>
    <div class="container">
        <img src="/video_feed" width="{{ width }}" height="{{ height }}">
    </div>
    <div class="stats">
        Status: LIVE | Device: Jetson/Pi | Protocol: MJPEG <br>
        Resolution: {{ width }}x{{ height }} | MJPEG Mode: {{ mjpg }}
    </div>
</body>
</html>
"""

def gen_frames(config):
    # Extract settings with safe defaults
    cam_conf = config.get('camera', {})
    idx = cam_conf.get('index', 0)
    width = cam_conf.get('width', 640)
    height = cam_conf.get('height', 480)
    fps = cam_conf.get('fps', 30)
    force_mjpg = cam_conf.get('force_mjpg', False)

    print(f"[System] Opening Camera {idx} at {width}x{height} (MJPEG={force_mjpg})...")

    # 1. Access Camera with V4L2 backend
    camera = cv2.VideoCapture(idx, cv2.CAP_V4L2)
    
    # 2. Apply Configuration
    if force_mjpg:
        camera.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc(*'MJPG'))
    
    camera.set(cv2.CAP_PROP_FRAME_WIDTH, width)
    camera.set(cv2.CAP_PROP_FRAME_HEIGHT, height)
    camera.set(cv2.CAP_PROP_FPS, fps)

    if not camera.isOpened():
        print("❌ Error: Could not open camera.")
        return

    while True:
        success, frame = camera.read()
        
        if not success:
            print("⚠️ Warning: Dropped frame (retrying...)")
            time.sleep(0.1)
            continue 
        
        # Timestamp Overlay
        cv2.putText(frame, f"REC: {time.time():.2f}", (20, height - 20), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/')
def index():
    # Pass config dimensions to HTML for correct aspect ratio
    config = load_config()
    w = config.get('camera', {}).get('width', 640)
    h = config.get('camera', {}).get('height', 480)
    m = config.get('camera', {}).get('force_mjpg', False)
    return render_template_string(PAGE_HTML, width=w, height=h, mjpg=m)

@app.route('/video_feed')
def video_feed():
    config = load_config()
    return Response(gen_frames(config), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)