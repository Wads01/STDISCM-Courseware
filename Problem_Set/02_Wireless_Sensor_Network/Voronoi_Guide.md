# Voronoi_Guide.md

This guide explains Qt from first principles and walks through how the Voronoi program uses core Qt concepts. It assumes you already understand the project layout, `main.cpp`, and how to build with CMake. The goal is to make you comfortable with Qt's main classes, the event loop, painting, widgets, layouts, and basic interactivity.

Contents
- Quick summary of the program and its GUI
- Qt fundamentals you need to know
  - `QApplication` and the event loop
  - `QMainWindow` and `QWidget`
  - Signals & slots
  - Layouts and widgets
  - Events and event handlers
  - Painting with `QPainter` and `QImage`
  - Coordinate mapping and device coordinates
- How the provided program uses these concepts (callouts to files/classes)
- Short recipes and common edits (make changes safely)
- Next steps and learning resources

Quick summary of the program
----------------------------
- The program parses a `config.txt` (you already know this) and produces a set of site coordinates and a distance threshold.
- It constructs a Qt GUI with a main window (`MainWindow`) that contains a top bar and a drawing canvas (`VoronoiWidget`).
- `VoronoiWidget` rasterizes the Voronoi diagram into a `QImage` once (or on resize/data change) and paints that image in its `paintEvent`.
- Each site has a white circular marker and a temperature label drawn with `QPainter` on top of the cached image.

Why this program is a good Qt learning example
- It uses basic widgets: `QPushButton`, `QWidget` and `QMainWindow`.
- It demonstrates signals/slots (button click -> file open) and event handling (paint and resize).
- It shows manual painting with `QPainter` and how to cache bitmaps with `QImage` for performance.

Qt fundamentals (detailed)
-------------------------

`QApplication` and the event loop
- `QApplication` is the central manager for any GUI app. Create exactly one instance in `main()` before creating widgets:

  `QApplication app(argc, argv);`

- Call `app.exec()` at the end of `main()` to run the GUI event loop. The event loop receives OS events (mouse, keyboard, paint, timers) and dispatches them to widgets.
- `update()` on a widget schedules a paint; it does not immediately call `paintEvent()`.

`QMainWindow` and `QWidget`
- `QMainWindow` is a convenience top-level window type providing menu bars, tool bars, status bars, and a central widget. It is used as the application's main window.
- `QWidget` is the base class for all GUI elements. Any custom drawable area should derive from `QWidget` (or `QFrame`) and implement `paintEvent()`.
- The drawing canvas `VoronoiWidget` in this project derives from `QWidget`.

Signals and slots (Qt's communication mechanism)
- Signals and slots decouple event producers from consumers.
- Example: connect a button's `clicked` signal to a slot that opens a file dialog:

  `connect(loadButton_, &QPushButton::clicked, this, &MainWindow::onLoadConfig);`

- Slots are just member functions (can be public, protected or private) and can be regular C++ functions if using the newer function-pointer connection syntax.

Layouts and widgets
- To arrange child widgets inside a parent, Qt uses layout managers such as `QVBoxLayout` and `QHBoxLayout`.
- Layouts automatically compute sizes and positions for their children when the window is resized.
- In the project, `MainWindow` creates a vertical layout with a top bar (containing the button) and the `VoronoiWidget` below it.

Events and event handlers
- Qt widgets receive events through virtual functions you can override, such as:
  - `void paintEvent(QPaintEvent* event)` — draw the widget contents with `QPainter`.
  - `void resizeEvent(QResizeEvent* event)` — called when the widget size changes.
  - `void mousePressEvent(QMouseEvent* event)`, `mouseMoveEvent`, `mouseReleaseEvent` — for mouse interaction.
- `update()` triggers a `paintEvent` in the next event loop iteration.
- Keep `paintEvent` fast. Do expensive computations in other functions and cache results.

Painting: `QPainter`, `QImage`, and device coordinates
- `QPainter` is the API to draw lines, shapes and text onto `QWidget`, `QImage`, `QPixmap`, and other paint devices.
- `QImage` is an in-memory buffer representing pixel data; modify it per-pixel if you need to compute an image (like the Voronoi raster) and then draw it with `painter.drawImage(0,0, cachedImage_)`.
- Typical pattern for heavy drawing work (used in this repo): compute a `QImage` once (in `regenerateImage()`), then in `paintEvent()` simply draw that image and overlay lightweight annotations (markers and text).

Device coordinates and mapping
- Widgets operate in device coordinates (pixels). Input data (site positions) may be in another coordinate system (e.g., an arbitrary domain).
- Map input coordinates to widget pixel coordinates using a scale and offset. The repo computes a uniform `scale` to keep aspect ratio and centers the set of sites inside the widget with margins.
- If you need high-DPI support, consider using device pixel ratio with `QPaintDevice::devicePixelRatio()` and `QImage::setDevicePixelRatio()` (Qt handles scaling on high-DPI displays).

Threading and responsiveness
- Painting and GUI operations must run on the main (GUI) thread.
- Long computations (e.g., regenerating a very large Voronoi image) should run on a background thread and then send the result to the GUI thread (via signals/slots or `QMetaObject::invokeMethod`) so the UI stays responsive.
- For many cores, you can generate the image in tiled sections concurrently and then compose into one `QImage` on the GUI thread.

Memory and performance notes
- `QImage` uses row-aligned buffers; writing through `scanLine()` is efficient.
- Avoid recomputing per-pixel ownership inside `paintEvent()` — do it in `regenerateImage()` and cache results.
- When implementing boundary drawing (thin lines where ownership changes), prefer an extra pass that marks edge pixels instead of per-pixel string comparisons every paint.

How the provided program uses the Qt concepts (file-by-file)
----------------------------------------------------------
- `main.cpp`:
  - Creates `QApplication` and `MainWindow`.
  - Passes parsed site data into `MainWindow` constructor.

- `Qt/include/MainWindow.hpp` and `Qt/src/MainWindow.cpp`:
  - `MainWindow` is a `QMainWindow` that creates a central `QWidget` containing a `QVBoxLayout`.
  - A top bar (`QWidget`) holds a `QPushButton`.
  - The `VoronoiWidget` is created and added to the vertical layout. On start, `MainWindow` calls `canvas_->setData(sites, temps, threshold)`.
  - The `Load Config` button is connected to `onLoadConfig()` which uses `QFileDialog` and `QTextStream` to read another config and call `canvas_->setData(...)` again.

- `Qt/include/VoronoiWidget.hpp` and `Qt/src/VoronoiWidget.cpp`:
  - `VoronoiWidget` holds the input and mapped site lists and a `QImage cachedImage_`.
  - `setData(...)` stores the data, generates deterministic, well-separated colors, calls `regenerateImage()` and `update()`.
  - `regenerateImage()` maps input coordinates to pixel coordinates and performs the brute-force per-pixel nearest-site computation to fill `cachedImage_`.
  - `paintEvent()` draws `cachedImage_` and overlays small white circular markers with black strokes and text labels near them.

Additional: How the Voronoi diagram is actually drawn (painting internals)
------------------------------------------------------------------------
This section explains in detail how the Voronoi diagram is generated and painted using Qt APIs (`QImage`, `QPainter`) and how the program maps input coordinates to widget pixels.

1) Why use `QImage` + `QPainter`?
- The computational part of producing a Voronoi diagram is per-pixel and potentially expensive. Doing this work inside `paintEvent()` would make UI stalls and repainting slow.
- The common pattern is: compute a `QImage` offline (on data change or resize), store it in `cachedImage_`, and in `paintEvent()` simply draw that image and lightweight overlays. This keeps painting fast and predictable.

2) Coordinate mapping (input -> widget pixels)
- Input coordinates are arbitrary. To draw them inside the widget we compute a uniform `scale` and offsets so the entire set fits within the widget with a margin while preserving aspect ratio.
- Steps:
  - Compute the bounding box (minX/minY, maxX/maxY) of input sites.
  - Compute available drawing area: widget size minus margins.
  - Compute `scale = min(availW / bboxWidth, availH / bboxHeight)`.
  - Compute offsets so the mapped sites are centered: `offsetX = margin + (availW - usedW)/2 - minX*scale` (similarly for Y).
  - Mapped pixel = `input * scale + offset`.
- The same `scale` factor is used to convert the config `distanceThreshold` into pixel-space for consistent interpretation.

3) Generating `cachedImage_` (rasterization)
- Allocate a `QImage` with the widget size and format `QImage::Format_RGB32`.
- Fill with a background color.
- For each pixel (x,y) in the image:
  - Find the nearest mapped site (compute squared Euclidean distance to every mapped site and pick the smallest). This is the brute-force approach: O(pixels * sites).
  - Compare the nearest distance with `thresholdSq` (threshold converted to pixel units squared). If above threshold, use a dark background color; otherwise use the site's color.
  - Write the pixel fast via `QRgb* scanLine = reinterpret_cast<QRgb*>(cachedImage_.scanLine(y)); scanLine[x] = color.rgb();`.

4) Painting overlays in `paintEvent()`
- `paintEvent()` draws the cached image with `QPainter::drawImage(0,0,cachedImage_)`.
- Then it draws small markers (white filled circles with black strokes) at each mapped site using `QPainter::drawEllipse()`.
- Labels (temperature values) are drawn with `QPainter::drawText()`. Text color is chosen for contrast against the region color using a brightness/luma calculation.

5) Threshold handling
- The config `distanceThreshold` is interpreted in the same coordinate units as input coordinates. It is converted to pixels by multiplying with the `scale` above and squared to compare to squared distances for efficiency.
- Pixels whose nearest distance exceeds the threshold are drawn with a uniform "out of range" color.

6) Caching and responsiveness
- `regenerateImage()` is called only when necessary (on `setData()` and on `resizeEvent()`). `paintEvent()` simply draws the precomputed image.
- For very large images or many sites, consider generating the image on a background thread and emitting the `QImage` to the GUI thread when ready.

7) Performance improvements (brief)
- Use spatial partitioning (grid / KD-tree) to reduce nearest-site checks.
- Render at a reduced resolution and upscale for fast interaction.
- Parallelize rasterization by processing scanline ranges concurrently, then combine into one `QImage`.

How to find the relevant code
- `regenerateImage()` and `rasterize()` in `Qt/src/VoronoiWidget.cpp` contain the mapping and per-pixel loop.
- `paintEvent()` in the same file shows how the `QImage` is drawn and how markers/labels are painted.

Recipes (common edits and where to make them)
--------------------------------------------
- Change marker size: edit `const double radius = 5.0;` in `VoronoiWidget::paintEvent`.
- Center labels on markers: use `QFontMetrics` to compute text size and draw with coordinates adjusted so the text anchor is centered.
- Increase color separation: change hue step or use a predefined palette. In `VoronoiWidget::setData()` the code uses a golden-angle hue step (137°) — increase saturation/value for stronger colors.
- Add borders between regions (fast raster approach): after computing owner for each pixel, run one more pass and mark pixels where any 4-connected neighbor has a different owner. Write a black color to those pixels (or overlay them in `paintEvent`).
- Make generation asynchronous: run `regenerateImage()` in a worker thread, emit a signal with the resulting `QImage`, and in the main thread `slot` assign it to `cachedImage_` and call `update()`.

Debugging tips
- If `paintEvent()` does not run, ensure the widget is visible and not covered by another widget. Call `update()` to request a repaint.
- If colors or markers don't appear, double-check that `mappedSites_` has been computed (it is computed in `regenerateImage()`) and that `setData()` is called.
- If the config file parsing fails when reloading, test reading the same file in `main.cpp` to confirm format.

Next steps for learning Qt
-------------------------
If you want a practical path to learn Qt quickly, try the following sequence:
1. Read the Qt documentation on `QWidget`, `QPainter` and event handling (official Qt docs are concise and include diagrams).
2. Make tiny edits to this project: change marker size, move label position, change colors.
3. Add a `mousePressEvent` handler to `VoronoiWidget` that prints the clicked point in widget coordinates — this helps you learn event coordinates and hit testing.
4. Implement image saving: call `cachedImage_.save("voronoi.png");` (try via a new menu action).
5. Explore `QThread` and move heavy computations off the GUI thread.

Helpful links
- Qt official docs: https://doc.qt.io
- `QWidget` reference: https://doc.qt.io/qt-6/qwidget.html
- `QPainter` reference: https://doc.qt.io/qt-6/qpainter.html
- Signals & slots overview: https://doc.qt.io/qt-6/signalsandslots.html

Closing notes
-------------
This guide focused on Qt concepts relevant to the project: application lifecycle, widget hierarchy, painting, events, and interactivity. The repository already follows good patterns for learning Qt: separation of UI code (`MainWindow`) from rendering logic (`VoronoiWidget`) and caching expensive work in `QImage`.

If you want, I can:
- Commit this Markdown into your repository (I will create `Voronoi_Guide.md`).
- Add small interactive examples (e.g., click to add a site).
- Implement region borders or asynchronous generation.

