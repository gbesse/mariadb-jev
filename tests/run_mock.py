import json
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path != "/v1/systemone" or self.headers.get("Authorization") != "Bearer test-key":
            self.send_error(403)
            return
        length = int(self.headers["Content-Length"])
        payload = json.loads(self.rfile.read(length))
        assert payload["model"] == "jev-1.13.0"
        assert payload["state"]["value"] == "The ending was good. Watch it!"
        assert list(payload["questions"]) == ["q0", "q1"]
        response = json.dumps({"answers": {
            "q0": {"type": "noul", "noul": 0.91},
            "q1": {"type": "noul", "noul": 0.84},
        }}).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(response)))
        self.end_headers()
        self.wfile.write(response)

    def log_message(self, *_args):
        pass


server = HTTPServer(("127.0.0.1", 18765), Handler)
thread = threading.Thread(target=server.serve_forever, daemon=True)
thread.start()
try:
    subprocess.run([sys.argv[1]], check=True)
finally:
    server.shutdown()
