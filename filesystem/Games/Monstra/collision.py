# Circles must have `position.x`, `position.y`, and `radius` attributes.

class Rectangle():
	def __init__(self, x, y, width, height):
		self.x = x
		self.y = y
		self.width = width
		self.height = height


class Circle():
	def __init__(self, x, y, radius):
		self.x = x
		self.y = y
		self.radius = radius


# Returns true if and only if the specified circle and rectangle are colliding.
def circleRect(circle, rect):
	# get rectangle edges
	left = rect.x - rect.width / 2
	right = rect.x + rect.width / 2
	top = rect.y - rect.height / 2
	bottom = rect.y + rect.height / 2

	# which rectangle edge are we closest to?
	# clamp circle center to closest edge
	# assume circle is inside the rectangle to start
	testX = circle.x
	testY = circle.y
	testX = max(testX, left)
	testX = min(testX, right)
	testY = max(testY, top)
	testY = min(testY, bottom)

	# if the distance is less than the radius, collision!
	return (circle.x - testX) ** 2 + (circle.y - testY) ** 2 <= circle.radius ** 2


# Returns true if and only if the specified rectangles are colliding.
def rectRect(rect1, rect2):
	# get rectangle edges
	left1 = rect1.x - rect1.width / 2
	right1 = rect1.x + rect1.width / 2
	top1 = rect1.y - rect1.height / 2
	bottom1 = rect1.y + rect1.height / 2

	left2 = rect2.x - rect2.width / 2
	right2 = rect2.x + rect2.width / 2
	top2 = rect2.y - rect2.height / 2
	bottom2 = rect2.y + rect2.height / 2

	return (
		left2 <= right1 and
		right2 >= left1 and
		top2 <= bottom1 and
		bottom2 >= top1
	)
