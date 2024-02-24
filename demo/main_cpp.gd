extends Node3D

var size = 64
var resolution = 0.0625

var noise_offset = Vector3()
var mesh_instance = null
var mesh : Mesh
var thread_generator : Thread
var mutex: Mutex

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
	mutex.unlock()
		
func generator():
	var started = Time.get_ticks_msec()
	var fps = 0.0
	var mg = MeshGeneratorExtension.new()
	
	while true:
		var current = Time.get_ticks_msec()
		if current - started >= 1000:
			print((current - started) / fps)
			started = current
			fps = 0
		
		var noise = FastNoiseLite.new()
		noise.offset = noise_offset
		noise_offset += Vector3(0.1, 0.1, 0.1)
		
		var values = mg.fillSphere(size)
		var m = mg.generate(size, resolution, values)
		fps += 1.0
		
		mutex.lock()
		mesh = m
		mutex.unlock()
