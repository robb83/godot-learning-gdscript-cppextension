extends Node3D

var size = 64
var resolution = 0.0625

var noise_offset = Vector3()
var mesh_instance = null
var mesh : Mesh
var thread_generator : Thread
var mutex: Mutex
var elapsed = 0

# Called when the node enters the scene tree for the first time.
func _ready():
	mesh_instance = MeshInstance3D.new()
	add_child(mesh_instance)
	
	mutex = Mutex.new()
	thread_generator = Thread.new()
	thread_generator.start(generator)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	mutex.lock()
	if mesh:
		mesh_instance.mesh = mesh
		mesh = null
		$LabelElapsed.text = "GDScript: %4.2f ms" % elapsed
	mutex.unlock()
		
func generator():
	var mg = MeshGeneratorScript.new()
	
	while true:
		var started = Time.get_ticks_msec()
		
		var noise = FastNoiseLite.new()
		noise.offset = noise_offset
		noise_offset += Vector3(0.1, 0.1, 0.1)
		
		var values = mg.fill_sphere(size)
		#var values = mc.fill_noise(size, noise)
		var m = mg.generate_mesh(size, resolution, values)
		
		var e = Time.get_ticks_msec() - started
		mutex.lock()
		mesh = m
		elapsed = e		
		mutex.unlock()
