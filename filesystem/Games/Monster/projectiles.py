from engine_nodes import (
	Sprite2DNode,
)
import gc


# Note: We make this a Sprite2DNode so we can get a tick()
class Projectiles(Sprite2DNode):
	def __init__(self):
		super().__init__(self)
		self.projectiles = []
		self.queueGC = False

	def tick(self, dt):
		# Run garbage collection to clean up removed projectiles
		if self.queueGC:
			self.queueGC = False
			print("Projectiles: gc.collect()")
			gc.collect()

	def add(self, projectile):
		self.projectiles.append(projectile)

	def remove(self, projectile):
		projectile.stop()
		self.projectiles.remove(projectile)
		self.queueGC = True

	def show(self):
		self.projectiles = []

	def hide(self):
		while len(self.projectiles) > 0:
			self.projectiles.pop().stop()
		self.queueGC = True
