import ctypes as c

class Hypercol:
	def createOrientation():
		pass
	def createPoint():
		pass
	def __init__(self, dim):
		global point, orientation
		self.DIM = dim
		l = c.cdll.LoadLibrary("hypercubeCollide/libhypercube.so")
		self.l = l
		l.getDim.restype = c.c_int
		reportedDim = l.getDim()
		if reportedDim != self.DIM:
			print("Hypercol construction failed! Requested DIM is ",self.DIM," but reported dim is ",reportedDim)

		class point(c.Structure):
			_fields_ = [("p", c.c_int*self.DIM)]#int p[DIM];
		class orientation(c.Structure):
			if self.DIM == 2:
				_fields_ = [("r", c.c_double)]#radians
			elif self.DIM == 3:
				_fields_ = [("r", c.c_double*4)]#quaternion
			else:
				print("Unimplemented for DIM ",self.DIM)
		def createPoint(coords):
			return point((c.c_int*self.DIM)(*(coords[:self.DIM])))#create a point. The argument defines a datatype (an array of DIM c_ints), and then calls its constructor with the unpacked and trimmed coords tuple
		if self.DIM == 2:
			def createOrientation(rad):
				return orientation(rad)
			self.createOrientation = createOrientation
		elif self.DIM == 3:
			def createOrientation(w,x,y,z):#quaternion
				return orientation((c.c_double*4)(w,x,y,z))
			self.createOrientation = createOrientation
		else:
			print("Unimplemented")
		self.createPoint = createPoint;
		
		l.inithypercube()
		l.newScene.argtypes = [c.c_int, c.c_int, c.c_char_p]
		l.newScene.restype = c.c_void_p
		l.freeScene.argtypes = [c.c_void_p]
		l.selectScene.argtypes = [c.c_void_p]
		l.loadOClass.argtypes = [c.c_char_p, c.c_char_p]
		l.calculateScene.argtypes = []
		l.getCollisions.argtypes = []
		l.getCollisions.restype = c.POINTER(c.c_int)#c.c_void_p
		l.newOInstance.argtypes = [c.c_char_p, c.c_int, point, orientation, c.c_float]
		l.newOInstance.restype = c.c_void_p
		l.addInstance.argtypes = [c.c_void_p]

	def __del__(self):
		self.l.cleanuphypercube()

	def newScene(self, collisionRules):
		s = ""
		for row in collisionRules:
			for col in row:
				s += str(col)
		return self.l.newScene(1, len(collisionRules), s.encode())

	def freeScene(self, scene):
		self.l.freeScene(scene)

	def selectScene(self, scene):
		self.l.selectScene(scene)

	def loadOClass(self, name, path):
		self.l.loadOClass(name.encode(), path.encode())

	def calculateScene(self):
		self.l.calculateScene()

	def getCollisions(self):
		return self.l.getCollisions()

	def newOInstance(self, name, coltype, loc, rot, scale):
		return self.l.newOInstance(name.encode(), coltype, loc, rot, scale)
	def addInstance(self, inst):
		self.l.addInstance(inst)
