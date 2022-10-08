#include "Renderer.h"
#include "Walnut/Random.h"

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize needed
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::SwitchLight()
{
	m_lpos++;
	m_lpos = (m_lpos % 10);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{

	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);

	const Sphere* closestSphere = nullptr;
	float minHitDistance = FLT_MAX;
	for (const Sphere& sphere : scene.Spheres)
	{
		glm::vec3 origin = ray.Origin - sphere.Position;

		// solve quadratic formula to find t hit distance
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// discriminant b^2 -4ac
		float discriminant = b * b - 4.0f * a * c;
		if (discriminant < 0.0f)
			continue;

		float closest = (-b - glm::sqrt(discriminant)) / (2 * a);
		if (closest < minHitDistance)
		{
			closestSphere = &sphere;
			minHitDistance = closest;
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(0, 0, 0, 1);


	
	glm::vec3 origin = ray.Origin - closestSphere->Position;

	glm::vec3 hitPoint = origin + ray.Direction * minHitDistance;
	glm::vec3 normal = glm::normalize(hitPoint);

	glm::vec3 LightDir = glm::normalize(glm::vec3(-1, -1, -1));


	float d = glm::max(glm::dot(normal, -LightDir), 0.0f);

	glm::vec3 resColor = closestSphere->Albedo;
	resColor *= d;
	return glm::vec4(resColor, 1.0f);
}


static uint32_t ConvertToRGBA(const glm::vec4& color)
{
	uint8_t r = (uint8_t)(color.r * 255.0f);
	uint8_t g = (uint8_t)(color.g * 255.0f);
	uint8_t b = (uint8_t)(color.b * 255.0f);
	uint8_t a = (uint8_t)(color.a * 255.0f);

	uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
	return result;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	// Render every pixel on viewPort

	const glm::vec3& rayOrigin = camera.GetPosition();

	Ray ray;
	ray.Origin = rayOrigin;


	uint32_t width = m_FinalImage->GetWidth();
	uint32_t height = m_FinalImage->GetHeight();
	for (uint32_t  y = 0; y < height; y++)
	{
		for (uint32_t x = 0; x < width; x++)
		{
			glm::vec2 coord = {(float)x / (float)width,(float)y / (float)height };

			const glm::vec3& rayDir = camera.GetRayDirections()[x+y*width];
			ray.Direction = rayDir;

			glm::vec4 color = TraceRay(scene,ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * width] = ConvertToRGBA(color);
		}
		
	}

	m_FinalImage->SetData(m_ImageData);

}