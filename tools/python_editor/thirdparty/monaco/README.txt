Offline Monaco support
----------------------

Place the monaco-editor distribution under this folder so that

  tools/python_editor/thirdparty/monaco/min/vs/loader.js

exists. You can obtain it from a machine with internet:

  npm i monaco-editor@0.51.0
  copy node_modules/monaco-editor/min to tools/python_editor/thirdparty/monaco/min

The editor will prefer this local path; if not present, it falls back to a CDN.

