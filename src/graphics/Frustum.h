#pragma once
#include <glm.hpp>
#include <array>

// Represents a mathematical clipping plane (Ax + By + Cz + D = 0)
struct Plane {
    glm::vec3 normal = {0.f, 0.f, 0.f};
    float distance = 0.f;

    // Normalizes plane equation coefficients for accurate signed distance calculations
    void Normalize() {
        float magnitude = glm::length(normal);
        normal /= magnitude;
        distance /= magnitude;
    }

    // Computes signed distance from a point to the plane
    float SignedDistance(const glm::vec3& point) const {
        return glm::dot(normal, point) + distance;
    }
};

class Frustum {
public:
    std::array<Plane, 6> planes; // Left, Right, Bottom, Top, Near, Far

    // Extracts 6 view frustum planes directly from the combined View-Projection matrix
    void Update(const glm::mat4& viewProj) {
        // Left Plane
        planes[0].normal = {viewProj[0][3] + viewProj[0][0], viewProj[1][3] + viewProj[1][0], viewProj[2][3] + viewProj[2][0]};
        planes[0].distance = viewProj[3][3] + viewProj[3][0];
        
        // Right Plane
        planes[1].normal = {viewProj[0][3] - viewProj[0][0], viewProj[1][3] - viewProj[1][0], viewProj[2][3] - viewProj[2][0]};
        planes[1].distance = viewProj[3][3] - viewProj[3][0];
        
        // Bottom Plane
        planes[2].normal = {viewProj[0][3] + viewProj[0][1], viewProj[1][3] + viewProj[1][1], viewProj[2][3] + viewProj[2][1]};
        planes[2].distance = viewProj[3][3] + viewProj[3][1];
        
        // Top Plane
        planes[3].normal = {viewProj[0][3] - viewProj[0][1], viewProj[1][3] - viewProj[1][1], viewProj[2][3] - viewProj[2][1]};
        planes[3].distance = viewProj[3][3] - viewProj[3][1];
        
        // Near Plane
        planes[4].normal = {viewProj[0][3] + viewProj[0][2], viewProj[1][3] + viewProj[1][2], viewProj[2][3] + viewProj[2][2]};
        planes[4].distance = viewProj[3][3] + viewProj[3][2];
        
        // Far Plane
        planes[5].normal = {viewProj[0][3] - viewProj[0][2], viewProj[1][3] - viewProj[1][2], viewProj[2][3] - viewProj[2][2]};
        planes[5].distance = viewProj[3][3] - viewProj[3][2];

        for (auto& plane : planes) {
            plane.Normalize();
        }
    }

    // Axis-Aligned Bounding Box (AABB) visibility test for chunk bounding boxes
    bool IsBoxVisible(const glm::vec3& minBounds, const glm::vec3& maxBounds) const {
        for (const auto& plane : planes) {
            // Check if all 8 vertices of the chunk AABB lie outside any clipping plane
            if (plane.SignedDistance(glm::vec3(minBounds.x, minBounds.y, minBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(maxBounds.x, minBounds.y, minBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(minBounds.x, maxBounds.y, minBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(maxBounds.x, maxBounds.y, minBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(minBounds.x, minBounds.y, maxBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(maxBounds.x, minBounds.y, maxBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(minBounds.x, maxBounds.y, maxBounds.z)) >= 0.0f) continue;
            if (plane.SignedDistance(glm::vec3(maxBounds.x, maxBounds.y, maxBounds.z)) >= 0.0f) continue;
            return false; // Entire bounding box is outside the frustum; skip rendering
        }
        return true; // Box intersects or is inside the frustum
    }
};