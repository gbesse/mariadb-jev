"""Run actual MariaDB SQL against the built UDF and a deterministic mock Jev API."""

import json
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from pathlib import Path


CONTAINER = "mariadb-jev-ci"


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        assert self.path == "/v1/systemone"
        assert self.headers.get("Authorization") == "Bearer test-key"
        payload = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
        assert payload["state"]["value"] == "The ending was good. Watch it!"
        answers = {
            key: {"type": "noul", "noul": 0.91 if key == "q0" else 0.84}
            for key in payload["questions"]
        }
        response = json.dumps({"answers": answers}).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(response)))
        self.end_headers()
        self.wfile.write(response)

    def log_message(self, *_args):
        pass


def run(*args, **kwargs):
    return subprocess.run(args, check=True, capture_output=True, text=True, **kwargs)


def mysql(sql):
    return run("docker", "exec", CONTAINER, "mariadb", "-uroot", "-pci-password",
               "-N", "-e", sql).stdout.strip()


def main():
    server = HTTPServer(("127.0.0.1", 18765), Handler)
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    try:
        run("docker", "run", "-d", "--network", "host", "--name", CONTAINER,
            "-e", "MARIADB_ROOT_PASSWORD=ci-password",
            "-e", "JEV_API_KEY=test-key",
            "-e", "JEV_API_URL=http://127.0.0.1:18765/v1/systemone",
            "mariadb:11.4")
        for _ in range(60):
            try:
                if mysql("SELECT 1") == "1":
                    break
            except subprocess.CalledProcessError:
                time.sleep(1)
        else:
            raise RuntimeError("MariaDB did not start")
        plugin_dir = mysql("SELECT @@plugin_dir")
        plugin_path = str(Path(plugin_dir) / "mariadb_jev.so")
        run("docker", "cp", "build/mariadb_jev.so",
            f"{CONTAINER}:{plugin_path}")
        dependencies = run("docker", "exec", CONTAINER, "ldd",
                           plugin_path).stdout
        if "not found" in dependencies:
            run("docker", "exec", CONTAINER, "sh", "-c",
                "apt-get update -qq && (apt-get install -y -qq libcurl4 libjansson4"
                " || apt-get install -y -qq libcurl4t64 libjansson4)")
        commands = Path("sql/install.sql").read_text()
        run("docker", "exec", "-i", CONTAINER, "mariadb", "-uroot", "-pci-password",
            input=commands)
        result = mysql("SELECT jev_all('The ending was good. Watch it!', "
                       "JSON_ARRAY('discusses ending','recommends movie')), "
                       "jev_any('The ending was good. Watch it!', "
                       "JSON_ARRAY('discusses ending','recommends movie')), "
                       "ROUND(jev_probability('The ending was good. Watch it!', "
                       "'discusses ending'), 2), "
                       "jev_all(NULL, JSON_ARRAY('anything'))")
        if result != "1\t1\t0.91\tNULL":
            raise AssertionError("unexpected MariaDB UDF result")
        print("MariaDB SQL integration passed")
    finally:
        subprocess.run(["docker", "rm", "-f", CONTAINER], capture_output=True)
        server.shutdown()


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(type(error).__name__ + ": " + str(error), file=sys.stderr)
        sys.exit(1)
